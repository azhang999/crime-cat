/* Game text drawing class
 * Ported and heavily adapted from Emma's game4 (text game) code:
 * https://github.com/emmaloool/InterviewTextGame
 */
#pragma once

#include <string>
#include <vector> 
#include <map>
#include <utility>
#include "data_path.hpp"


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
	// void render_text(int scene_id, int text_id, float x, float y, glm::vec3 color);
	void render_text_buffer(uint32_t hb_index, float x, float y, glm::vec3 color, uint8_t font_id);
	void draw_text(float x, float y, glm::vec3 color);

	// -------- Screen Text Initialization/Maintenance --------
	// struct ScreenText {
	// 	size_t id;								// ID of current screen
		std::vector<uint8_t> font_ids;				// Font associated with each text line
		std::vector<std::vector<char>> lines;	// Text lines
	// };
	// std::vector<ScreenText> screen_texts;


	std::vector<hb_buffer_t *> hb_buffers;

	void init_state(std::string script_path);
	void fill_state();
	void add_text_to_HBbuf(std::vector<char> text, uint8_t font_id);

	// -------- Font Character Rendering --------

	// Character->character map and glyph struct from https://learnopengl.com/In-Practice/Text-Rendering
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	std::string belligerent_font_path = data_path("./font/BelligerentMadness/belligerent.ttf");
	std::string blok_font_path = data_path("./font/Blokletters-Potlood/Blokletters-Potlood.ttf");
	std::string nunito_font_path = data_path("./font/nunito/Nunito-Regular.ttf");
	
	typedef enum FontID_t {
		Belligerent_title = 0,
		Belligerent_header = 1,
		Blokletters = 2,
		Nunito = 3
	} FontID;

	struct Font {
		Font(std::string font_path_, int height_, int offset_) : path(font_path_), height(height_), offset(offset_) {};

		std::string path;
		int height;
		int offset;
		FontID id;
		FT_Library lib;
		FT_Face face;
		hb_font_t * hb_font;
		std::map<FT_ULong, Character> characters; // Reuse characters previously rendered
	};
	std::vector<Font> fonts;

	// std::string path;
	// int height; 
	// FontID id;
	// FT_Library lib;
	// FT_Face face;
	// hb_font_t * hb_font;
	// std::map<FT_ULong, Character> characters;

	// -------- Drawing Constants --------
	const float LEFT_X = 80.0f;
	const float TOP_Y = 650.0f;

	const float CENTER_X = 625.0f;
	const float CENTER_Y = 350.0f;

};