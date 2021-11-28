#include "GameText.hpp"

#include "TextTextureProgram.hpp" // Load to use for text
#include "read_write_chunk.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

GLuint VAO, VBO;

GameText::GameText() {

}

void GameText::setup(std::string font_path_, std::string script_path_) {
    if (init) return;
    script_path = script_path_;

    // Setup FT, HB font - code from https://learnopengl.com/In-Practice/Text-Rendering
	{
		if (FT_Init_FreeType(&font_lib)) throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
		if (FT_New_Face(font_lib, const_cast<char *>(font_path_.c_str()), 0, &font_face)) throw std::runtime_error("ERROR::FREETYPE: Failed to load font");		// TODO use datapath for this please...
		FT_Set_Char_Size(font_face, 0, 1800, 0,0);
		if (FT_Load_Char(font_face, 'X', FT_LOAD_RENDER)) throw std::runtime_error("ERROR::FREETYPE: Failed to load Glyph");

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
		
		/* Create hb-ft font - needs to last the lifetime of the program */
		hb_font = hb_ft_font_create_referenced(font_face);
	}

	// Back to following https://learnopengl.com/In-Practice/Text-Rendering - 
	// configure VAO/VBO for texture quads
	// -----------------------------------
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	
    // Fill game state from text script
	fill_state();	
}

GameText::~GameText() {
    if (init) {
        // Destroy FT, HB objects in reverse order of creation
        // for (auto buf : hb_buffers) hb_buffer_destroy(hb_buffer);
        // hb_buffers.clear();
        hb_buffer_destroy(hb_buffer);

        hb_font_destroy(hb_font);
        FT_Done_Face(font_face);
        FT_Done_FreeType(font_lib);
    }
}

void GameText::fill_state() {
    // Read state from file
    { 
        std::fstream txt_file;
        std::string line;
        txt_file.open(script_path, std::ios::in);
        if (txt_file.is_open()) {
            while (getline(txt_file, line)) {
                std::cout<< line <<std::endl;

                // Read text paragraphs as a sequence of lines, so we can't just loop over all lines
                uint8_t num_lines = stoi(line); 	// tells us how many phase text lines to read

                // Add lines in this paragraph
                for (uint8_t i = 0; i < num_lines; i++) {
                    getline(txt_file, line);
                    std::cout<< line <<std::endl;
        
                    // TODO: case for font, add character in front of text line to denote which font
                    // size_t offset = 0;
                    // if (line[0] == '#') {
                    // 	offset = 1;
                    // 	fonts.push_back(0);
                    // }
                    // else {
                    // 	phase.fonts.push_back(0);
                    // }

                    std::vector<char> line_chars(line.begin(), line.end());
                    add_text_to_HBbuf(line_chars, hb_font);
                }

                getline(txt_file,line);
            }
        }
    }
}

void GameText::add_text_to_HBbuf(std::vector<char> text, hb_font_t *font) {
    // Setup HB buffer - code from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
    // and http://www.manpagez.com/html/harfbuzz/harfbuzz-2.3.1/ch03s03.php

    /* Create hb-buffer and populate. */
    hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8(hb_buffer, &text[0], (int)text.size(), 0, (int)text.size());

    /* Guess the script/language/direction */
    hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(hb_buffer, hb_language_from_string("en",-1));

    /* Shape it! */
    hb_shape(font, hb_buffer, NULL, 0);

    // hb_buffers.push_back(hb_buffer);
}

void GameText::render_text(float x, float y, glm::vec3 color) {
	glDisable(GL_DEPTH_TEST);

	// Render text - again following https://learnopengl.com/In-Practice/Text-Rendering, function RenderText()
	// Setup character render state to render text 
	auto &Characters = characters;
	FT_Face face = font_face;
	
	// Our render state is associated with the shader program color_texture_program
	glUseProgram(text_texture_program->program);
	// According to the text rendering tutorial:
	// "For rendering text we (usually) do not need perspective, and using an 
	// orthographic projection matrix also allows us to specify all vertex coordinates in screen coordinates"
	glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
	// (Again) according to the text rendering tutorial:
	// "We set the projection matrix's bottom parameter to 0.0f 
	// and its top parameter equal to the window's height. 
	// The result is that we specify coordinates with y values ranging from 
	// the bottom part of the screen (0.0f) to the top part of the screen (600.0f). 
	// This means that the point (0.0, 0.0) now corresponds to the bottom-left corner.
	// So we have to pass a pointer to projection[0][0] to the program's projection matrix
	// as "values" parameter of glUniformMatrix4fv
	glUniformMatrix4fv(text_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, &projection[0][0]);
	// Since the shader is programmed to take in a text color as uniform, we also need to set this now
	glUniform3f(text_texture_program->Color_vec3, color.x, color.y, color.z);		// TODO dynamically set colors here, read in as game struct
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Harbuzz buffer processing code from http://www.manpagez.com/html/harfbuzz/harfbuzz-2.3.1/ch03s03.php
	// Get glyph and position information
	unsigned int glyph_count;
	hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);

	// Iterate through all the necessary glyphs to render in this buffer
    for (uint8_t i = 0; i < glyph_count; i++)
    {
		// In this game, we generate text on the fly
		// So this is the first time we are potentially seeing new characters
		// But we don't want to have to render them each time we see them
		// So...

		// From https://learnopengl.com/In-Practice/Text-Rendering
		// "For each character, we generate a texture and store its relevant data 
		// into a Character struct that we add to the Characters map. 
		// This way, all data required to render each character is stored for later use."
		// So we need to lookup if this character has been previously stored in the character map
		FT_ULong c = glyph_info[i].codepoint;
		if (Characters.find(c) == Characters.end()) { // Character not previously rendered
			// Load character glyph
			if (FT_Load_Glyph(face, c, FT_LOAD_RENDER))
				std::runtime_error("ERROR::FREETYPE: Failed to load Glyph");
			
			// Generate texture 
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Now store character for later use
			Character character = {
				texture, 
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				(unsigned int) face->glyph->advance.x
			};
			Characters.insert(std::pair<FT_ULong, Character>(c, character));
		}

		// Now character should be present in the map at the codepoint!
        Character ch = Characters[c];

        float xpos = x + ((float)ch.Bearing.x);
        float ypos = y - (((float)ch.Size.y) - ((float)ch.Bearing.y));

        float w = ch.Size.x;
        float h = ch.Size.y;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)
    }

	glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}