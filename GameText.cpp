/* Game text drawing class
 * Ported and heavily adapted from Emma's game4 (text game) code:
 * https://github.com/emmaloool/InterviewTextGame
 */

#include "GameText.hpp"

#include "TextTextureProgram.hpp" // Load to use for text
#include "read_write_chunk.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

GLuint VAO, VBO;

GameText::GameText() {  
	{ // For each of the fonts, setup FT, HB font - code from https://learnopengl.com/In-Practice/Text-Rendering
		fonts.push_back(Font(belligerent_font_path, 9000, 150));
		fonts.push_back(Font(belligerent_font_path, 3200, 50));
		fonts.push_back(Font(nunito_font_path, 2400, 100));
		fonts.push_back(Font(nunito_font_path, 1600, 30));

        for (auto &font: fonts) {
            if (FT_Init_FreeType(&font.lib)) throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
            if (FT_New_Face(font.lib, const_cast<char *>(font.path.c_str()), 0, &(font.face))) throw std::runtime_error("ERROR::FREETYPE: Failed to load font");		// TODO use datapath for this please...
            FT_Set_Char_Size(font.face, 0, font.height, 0,0);
            if (FT_Load_Char(font.face, 'X', FT_LOAD_RENDER)) throw std::runtime_error("ERROR::FREETYPE: Failed to load Glyph");

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
            
            /* Create hb-ft font - needs to last the lifetime of the program */
            font.hb_font = hb_ft_font_create_referenced(font.face);
        }
	}
	
	{ // configure VAO/VBO for texture quads
        // Back to following https://learnopengl.com/In-Practice/Text-Rendering - 
        // -----------------------------------
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
}

GameText::~GameText() {
    // Destroy FT, HB objects in reverse order of creation
    for (auto buf : hb_buffers)  {
        hb_buffer_destroy(buf);
    }
    hb_buffers.clear();

    for (auto font : fonts) {
        hb_font_destroy(font.hb_font);
        FT_Done_Face(font.face);
        FT_Done_FreeType(font.lib);
    }
}

void GameText::init_state(std::string script_path) {
    std::fstream txt_file;
    std::string line;
    txt_file.open(script_path, std::ios::in);
    if (txt_file.is_open()) {
        while (getline(txt_file, line)) {
            std::cout << line << std::endl;
            // Read text paragraphs as a sequence of lines, so we can't just loop over all lines
            auto f_i = line.find(' ');
            uint8_t num_lines = stoi(line.substr(0, f_i));
            uint8_t font_type = stoi(line.substr(f_i));

            // Add lines in this paragraph
            for (uint8_t i = 0; i < num_lines; i++) {
                getline(txt_file, line);    
                font_ids.push_back(font_type);
                std::vector<char> line_chars(line.begin(), line.end());
                lines.push_back(line_chars);
            }

            getline(txt_file,line);
        }
    }

    if (PLAYMODE) {
        SCORE = lines.size();
        TIME = lines.size() + 1;
        COLLISION = lines.size() + 2;
    }

    std::cout << "LINES SIZE: " << line.size() << std::endl;
}

void GameText::fill_state() {
    hb_buffers.clear();
    
    for (size_t i = 0; i < lines.size(); i++) {
        add_text_to_HBbuf(lines[i], font_ids[i]);
    }

}

void GameText::edit_state(size_t buf_id, std::string new_line, glm::vec3 color) {
    if (PLAYMODE) {
        // lines[buf_id].clear();
        // for (auto c : new_chars) lines[buf_id].push_back(c);

        std::vector<char> new_chars(new_line.begin(), new_line.end());

        if (buf_id == SCORE) score = new_chars;
        else if (buf_id == TIME) time = new_chars;
        else if (buf_id == COLLISION) collision = new_chars;

        if (color != glm::vec3(-1.0f)) {
            if (buf_id == SCORE) score_color = color;
            else if (buf_id == TIME)  time_color = color;
            else if (buf_id == COLLISION) collision_color = color;
        }
    }
}

void GameText::update_state() {
    if (PLAYMODE) fill_state();

    // Add special lines
    add_text_to_HBbuf(score, 1);
    add_text_to_HBbuf(time, 1);
    add_text_to_HBbuf(collision, 1);
}


void GameText::add_text_to_HBbuf(std::vector<char> text, uint8_t font_id) {
    // Setup HB buffer - code from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
    // and http://www.manpagez.com/html/harfbuzz/harfbuzz-2.3.1/ch03s03.php

    /* Create hb-buffer and populate. */
    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, &text[0], (int)text.size(), 0, (int)text.size());

    /* Guess the script/language/direction */
    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buf, hb_language_from_string("en",-1));

    /* Shape it! */
    hb_shape(fonts[font_id].hb_font, buf, NULL, 0);

    hb_buffers.push_back(buf);
}

void GameText::render_text_buffer(uint32_t hb_index, float x, float y, glm::vec3 color, uint8_t font_id) {
	glDisable(GL_DEPTH_TEST);

	// Render text - again following https://learnopengl.com/In-Practice/Text-Rendering, function RenderText()
	// Setup character render state to render text 
	auto &Characters = fonts[font_id].characters;
	FT_Face face = fonts[font_id].face;
	
	// Our render state is associated with the shader program color_texture_program
	glUseProgram(text_texture_program->program);
	// According to the text rendering tutorial:
	// "For rendering text we (usually) do not need perspective, and using an 
	// orthographic projection matrix also allows us to specify all vertex coordinates in screen coordinates"
	glm::mat4 projection = glm::ortho(0.0f, 1200.0f, 0.0f, 900.0f);
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

    // Get current HB buffer
	hb_buffer_t *cur_buffer = hb_buffers[hb_index];

	// Harbuzz buffer processing code from http://www.manpagez.com/html/harfbuzz/harfbuzz-2.3.1/ch03s03.php
	// Get glyph and position information
	unsigned int glyph_count;
	hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(cur_buffer, &glyph_count);

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

void GameText::draw_text(float x, float y, glm::vec3 color) {
    auto height_offset = 50;

    if (PLAYMODE) {
        // TODO: really not sure why 3 is the offset but oh well!
        for (size_t i = 0; i < lines.size() - 3; i++) {
            render_text_buffer(i, x, y - height_offset, color, font_ids[i]);
            height_offset += fonts[font_ids[i]].offset;
        }

        render_text_buffer(SCORE, score_loc.x, score_loc.y, score_color, 1);
        render_text_buffer(TIME, time_loc.x, time_loc.y, time_color, 1);
        render_text_buffer(COLLISION, collision_loc.x, collision_loc.y, collision_color, 1);
        
        if (GAMEOVER) {
            size_t i = lines.size() - 3;
            render_text_buffer(i, x, y - height_offset, glm::vec3(0.1f, 0.1f, 0.1f), font_ids[i]);
            height_offset += fonts[font_ids[i]].offset + 100.0f;

            i = lines.size() - 1;
            render_text_buffer(i, x, y - height_offset, glm::vec3(0.4f, 0.1f, 0.1f), font_ids[i]);
        }
    }
    else {
        for (size_t i = 0; i < lines.size(); i++) {
            render_text_buffer(i, x, y - height_offset, color, font_ids[i]);
            height_offset += fonts[font_ids[i]].offset;
        }
    }
}
