#include "InstructMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "load_save_png.hpp"
#include "data_path.hpp"

using namespace std;

InstructMode::InstructMode(std::shared_ptr< Mode > const &next_mode_) : next_mode(next_mode_){
    //---------- the bulk of the following opengl code is from game0 ----------
	
    //----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of InstructMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1,1);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

    { //------ Loading splash screen as background ------
        // Based on loading code from https://github.com/kjannakh/15-466-f20-base4/blob/master/PlayMode.cpp
        glm::uvec2 imgbg_size(0,0);
        std::vector< glm::u8vec4 > imgbg_data;
        load_png(data_path(imgbg_path), &imgbg_size, &imgbg_data, LowerLeftOrigin);

        // Repeat texture setup code
        glGenTextures(1, &splash_tex);
        glBindTexture(GL_TEXTURE_2D, splash_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgbg_size.x, imgbg_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgbg_data.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
    }
}

InstructMode::~InstructMode() {

	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
    splash_tex = 0;
}

bool InstructMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	// if (evt.type == SDL_MOUSEMOTION) {
	// 	//convert mouse from window pixels (top-left origin, +y is down) to clip space ([-1,1]x[-1,1], +y is up):
	// 	glm::vec2 clip_mouse = glm::vec2(
	// 		(evt.motion.x + 0.5f) / window_size.x * 2.0f - 1.0f,
	// 		(evt.motion.y + 0.5f) / window_size.y *-2.0f + 1.0f
	// 	);
	// }
    // TODO: temporary, until clicking on button regions is enabled
    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_p) {
			Mode::set_current(next_mode);
			return true;
		}
    }

	return false;
}

void InstructMode::update(float elapsed) {

	// TODO: based on position of clicks

}

void InstructMode::draw(glm::uvec2 const &drawable_size) {

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	// std::vector< Vertex > vertices;

	// //inline helper function for rectangle drawing:
	// auto setup_rect_vertices = [&](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
	// 	//draw rectangle as two CCW-oriented triangles:
	// 	// vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 1.0f));
	// 	// vertices.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
	// 	// vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));

	// 	// vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
	// 	// vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
	// 	// vertices.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
	// };

    // TODO draw buttons with text or something

    //---- Drawing the background image ----
    // References code to draw background image texture as two screen-sized triangles:
    // https://github.com/kjannakh/15-466-f20-base4/blob/master/PlayMode.cpp
    { 
        //clear the color buffer:
        glClearColor(0.f,0.f,0.f,0.f);
        glClear(GL_COLOR_BUFFER_BIT);

        //use alpha blending:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //don't use the depth test:
        glDisable(GL_DEPTH_TEST);

        std::vector< Vertex > vertices;
        glm::u8vec4 white_color = glm::u8vec4(255, 255, 255, 255);
        vertices.emplace_back(Vertex(glm::vec3(0.0f, float(drawable_size.y), 0.0f), white_color, glm::vec2(0.0f, 1.0f)));
        vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), float(drawable_size.y), 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
        vertices.emplace_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));

        vertices.emplace_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f), white_color, glm::vec2(0.0f, 0.0f)));
        vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), float(drawable_size.y), 0.0f), white_color, glm::vec2(1.0f, 1.0f)));
        vertices.emplace_back(Vertex(glm::vec3(float(drawable_size.x), 0.0f, 0.0f), white_color, glm::vec2(1.0f, 0.0f)));

        //upload vertices to vertex_buffer:
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //set color_texture_program as current program:
        glUseProgram(color_texture_program.program);

        //upload OBJECT_TO_CLIP to the proper uniform location:
        glm::mat4 projection = glm::ortho(0.0f, float(drawable_size.x), 0.0f, float(drawable_size.y));
        glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(projection));

        //use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
        glBindVertexArray(vertex_buffer_for_color_texture_program);

        //bind the solid white texture to location zero so things will be drawn just with their colors:
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, splash_tex);

        //run the OpenGL pipeline:
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

        //unbind the solid white texture:
        glBindTexture(GL_TEXTURE_2D, 0);

        //reset vertex array to none:
        glBindVertexArray(0);

        //reset current program to none:
        glUseProgram(0);
        

        GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.
    }
}
