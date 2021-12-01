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
	void render_text_buffer(size_t hb_index, float x, float y, glm::vec3 color, uint8_t font_id);
	void draw_text(float x, float y, glm::vec3 color);

	// -------- Text State Routines ---------
	void init_state(std::string script_path);
	void fill_state();
	void edit_state(size_t buf_id, std::string new_line, glm::vec3 color = glm::vec3(-1.0f));
	void update_state();
	void add_text_to_HBbuf(std::vector<char> text, uint8_t font_id);

	// -------- Screen Text Initialization/Maintenance --------
	// struct ScreenText {
	// 	size_t id;								// ID of current screen
	std::vector<uint8_t> font_ids;				// Font associated with each text line
	std::vector<std::vector<char>> lines;	// Text lines
	// };
	// std::vector<ScreenText> screen_texts;
	std::vector<hb_buffer_t *> hb_buffers;

	// ------ Special fields for PlayMode-modifiable text ------
	bool PLAYMODE = false;
	bool GAMEOVER = false;
	std::vector<char> score = {'0'};
	std::vector<char> time = {'1',':','0','0'};
	std::vector<char> collision = {' '};

	size_t SCORE = 0;
	size_t TIME = 1;
	size_t COLLISION = 2;

	glm::vec2 score_loc = glm::vec2(250.0f, 800.0f);
	glm::vec2 time_loc = glm::vec2(1000.0f, 800.0f);
	glm::vec2 collision_loc = glm::vec2(250.0f, 750.0f);

	glm::vec3 score_color = glm::vec3(0.4f, 0.4f, 1.0f);
	glm::vec3 time_color = glm::vec3(0.2f, 0.8f, 0.2f);
	glm::vec3 collision_color = glm::vec3(1.0f, 0.8f, 0.8f);

	// -------- Font Character Rendering --------

	// Character->character map and glyph struct from https://learnopengl.com/In-Practice/Text-Rendering
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	std::string belligerent_font_path = data_path("./font/BelligerentMadness/belligerent.ttf");
	std::string nunito_font_path = data_path("./font/nunito/Nunito-Regular.ttf");
	
	// Fonts are associated with a typeface and a size style
	// The font for a particular portion of text can be specified in the text file
	typedef enum FontID_t {
		Belligerent_title = 0,
		Belligerent_header = 1,
		Belligerent_text = 2,

	} FontID;

	struct Font {
		Font(std::string font_path_, int height_, float offset_) : path(font_path_), height(height_), offset(offset_) {};

		std::string path;
		int height;
		float offset;
		FontID id;
		FT_Library lib;
		FT_Face face;
		hb_font_t * hb_font;
		std::map<FT_ULong, Character> characters; // Reuse characters previously rendered
	};
	std::vector<Font> fonts;

	// -------- Drawing Constants --------
	const float LEFT_X = 80.0f;
	const float TOP_Y = 830.0f;
	const float RIGHT_X  = 830.0f;
	// const float BOT_Y = 600.0f;

	const float CENTER_X = 625.0f;
	const float CENTER_Y = 350.0f;

};