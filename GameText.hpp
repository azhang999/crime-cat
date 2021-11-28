/* Game text drawing class
 * Ported and heavily adapted from Emma's game4 (text game) code:
 * https://github.com/emmaloool/InterviewTextGame
 */
#pragma once

#include <string>
#include <vector> 
#include <map>
#include <utility>

#include <glm/glm.hpp>
#include <ft2build.h>
#include <hb.h>
#include <hb-ft.h>

#include FT_FREETYPE_H


struct GameText {
	// GameText(std::string font_path, std::string script_path);
	GameText();
	~GameText();

	// -------- Text Drawing Routine --------
	void render_text(float x, float y, glm::vec3 color);

	bool init = false;
	void setup(std::string font_path, std::string script_path);

	void fill_state();
	void add_text_to_HBbuf(std::vector<char> text, hb_font_t *font);
	

	// -------- Text State --------
	// std::vector<uint8_t> fonts;						// Fonts for each line
	// std::vector<std::vector<char>> lines;			// Text to display

	const float TEXT_START_X = 0.0f;
	const float TEXT_START_Y = 500.0f;

	// -------- Character Rendering --------

	// Character->character map and glyph struct from https://learnopengl.com/In-Practice/Text-Rendering
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	// TODO: just one font for now
	std::map<FT_ULong, Character> characters;	// Reuse characters previously rendered

	FT_Library font_lib;
	FT_Face font_face;
	hb_font_t *hb_font;

	// Stores text to render for a given scene
	hb_buffer_t * hb_buffer;

	std::string script_path;
};