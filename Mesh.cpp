#include "Mesh.hpp"
#include "read_write_chunk.hpp"

#include <glm/glm.hpp>

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <cstddef>

MeshBuffer::MeshBuffer(std::string const &pnct_name, std::string const &bb_name) {
	glGenBuffers(1, &buffer);

	std::ifstream pnct_file(pnct_name, std::ios::binary);

	GLuint total = 0;

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 3*4+3*4+4*1+2*4, "Vertex is packed.");
	std::vector< Vertex > vertex_data;

	//read + upload vertex_data chunk:
	if (pnct_name.size() >= 5 && pnct_name.substr(pnct_name.size()-5) == ".pnct") {
		read_chunk(pnct_file, "pnct", &vertex_data);

		//upload vertex_data:
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(Vertex), vertex_data.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		total = GLuint(vertex_data.size()); //store total for later checks on index

		//store attrib locations:
		Position = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Position));
		Normal = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Normal));
		Color = Attrib(4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, Color));
		TexCoord = Attrib(2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, TexCoord));
	} else {
		throw std::runtime_error("Unknown pnct_file type '" + pnct_name + "'");
	}

	std::vector< char > strings;
	read_chunk(pnct_file, "str0", &strings);

	{ //read index chunk, add to meshes:
		struct IndexEntry {
			uint32_t name_begin, name_end;
			uint32_t vertex_begin, vertex_end;
		};
		static_assert(sizeof(IndexEntry) == 16, "Index entry should be packed");

		std::vector< IndexEntry > index;
		read_chunk(pnct_file, "idx0", &index);

		for (auto const &entry : index) {
			if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings.size())) {
				throw std::runtime_error("index entry has out-of-range name begin/end");
			}
			if (!(entry.vertex_begin <= entry.vertex_end && entry.vertex_end <= total)) {
				throw std::runtime_error("index entry has out-of-range vertex start/count");
			}
			std::string name(&strings[0] + entry.name_begin, &strings[0] + entry.name_end);
			Mesh mesh;
			mesh.type = GL_TRIANGLES;
			mesh.start = entry.vertex_begin;
			mesh.count = entry.vertex_end - entry.vertex_begin;
			for (uint32_t v = entry.vertex_begin; v < entry.vertex_end; ++v) {
				mesh.min = glm::min(mesh.min, vertex_data[v].Position);
				mesh.max = glm::max(mesh.max, vertex_data[v].Position);
			}
			bool inserted = meshes.insert(std::make_pair(name, mesh)).second;
			if (!inserted) {
				std::cerr << "WARNING: mesh name '" + name + "' in pnct_name '" + pnct_name + "' collides with existing mesh." << std::endl;
			}
		}
	}

	if (pnct_file.peek() != EOF) {
		std::cerr << "WARNING: trailing data in mesh pnct_file '" << pnct_name << "'" << std::endl;
	}

    std::ifstream bb_file(bb_name, std::ios::binary);
    static_assert(sizeof(BoundBox) == 3*4*8, "BoundBox is packed.");
    std::vector< BoundBox > bound_box_data;

    //read + upload vertex_data chunk:
	if (bb_name.size() >= 9 && bb_name.substr(bb_name.size()-9) == ".boundbox") {
		read_chunk(bb_file, "pnct", &bound_box_data);
	} else {
		throw std::runtime_error("Unknown bb_file type '" + bb_name + "'");
	}

    std::vector< char > bb_strings;
	read_chunk(bb_file, "str0", &bb_strings);

	{ //read index chunk, add to meshes:
		struct IndexEntry {
			uint32_t name_begin, name_end;
		};
		static_assert(sizeof(IndexEntry) == 8, "Index entry should be packed");

		std::vector< IndexEntry > index;
		read_chunk(bb_file, "idx0", &index);
        int i = 0;
		for (auto const &entry : index) {
			if (!(entry.name_begin <= entry.name_end && entry.name_end <= bb_strings.size())) {
				throw std::runtime_error("index entry has out-of-range name begin/end");
			}
			std::string name(&bb_strings[0] + entry.name_begin, &bb_strings[0] + entry.name_end);
            bound_boxes[name] = bound_box_data[i];
			i++;
		}
	}

	if (bb_file.peek() != EOF) {
		std::cerr << "WARNING: trailing data in bb_file '" << bb_name << "'" << std::endl;
	}

	/* //DEBUG:
	std::cout << "File '" << pnct_name << "' contained meshes";
	for (auto const &m : meshes) {
		if (&m.second == &meshes.rbegin()->second && meshes.size() > 1) std::cout << " and";
		std::cout << " '" << m.first << "'";
		if (&m.second != &meshes.rbegin()->second) std::cout << ",";
	}
	std::cout << std::endl;
	*/
}

const Mesh &MeshBuffer::lookup(std::string const &name) const {
	auto f = meshes.find(name);
	if (f == meshes.end()) {
		throw std::runtime_error("Looking up mesh '" + name + "' that doesn't exist.");
	}
	return f->second;
}

const BoundBox &MeshBuffer::lookup_bound_box(std::string const &name) const {
	auto f = bound_boxes.find(name);
	if (f == bound_boxes.end()) {
		throw std::runtime_error("Looking up bound_box '" + name + "' that doesn't exist.");
	}
	return f->second;
}

GLuint MeshBuffer::make_vao_for_program(GLuint program) const {
	//create a new vertex array object:
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Try to bind all attributes in this buffer:
	std::set< GLuint > bound;
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	auto bind_attribute = [&](char const *name, MeshBuffer::Attrib const &attrib) {
		if (attrib.size == 0) return; //don't bind empty attribs
		GLint location = glGetAttribLocation(program, name);
		if (location == -1) return; //can't bind missing attribs
		glVertexAttribPointer(location, attrib.size, attrib.type, attrib.normalized, attrib.stride, (GLbyte *)0 + attrib.offset);
		glEnableVertexAttribArray(location);
		bound.insert(location);
	};
	bind_attribute("Position", Position);
	bind_attribute("Normal", Normal);
	bind_attribute("Color", Color);
	bind_attribute("TexCoord", TexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Check that all active attributes were bound:
	GLint active = 0;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &active);
	assert(active >= 0 && "Doesn't makes sense to have negative active attributes.");
	for (GLuint i = 0; i < GLuint(active); ++i) {
		GLchar name[100];
		GLint size = 0;
		GLenum type = 0;
		glGetActiveAttrib(program, i, 100, NULL, &size, &type, name);
		name[99] = '\0';
		GLint location = glGetAttribLocation(program, name);
		if (!bound.count(GLuint(location))) {
			throw std::runtime_error("ERROR: active attribute '" + std::string(name) + "' in program is not bound.");
		}
	}

	return vao;
}
