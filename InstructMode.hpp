/*
 * Untitled Cat Game instruction screen
 * This code is loosely ported from 15-466 game0: https://github.com/15-466/15-466-f21-base0 
 * Implementation of helper lambdas in draw() is from Emma's game0 base code: https://github.com/emmaloool/15-466-f21-base0
 */

#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <functional>

#include <math.h>
#include <stdlib.h>
#include <time.h>

struct InstructMode : Mode {
    // Passes a pointer to the mode that will be invoked when this one is over/skipped
    // Adapted from ttps://raw.githubusercontent.com/15-466/15-466-f21-intro/main/GP21IntroMode.hpp:
	InstructMode(std::shared_ptr< Mode > const &next_mode);
	virtual ~InstructMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    //------ After splash screen, loads instruction screen ------
    std::shared_ptr< Mode > next_mode;

    //----- Background image assets ------
    std::string imgbg_path = "./img/instructions.png";
    GLuint splash_tex = 0;

	//----- opengl assets / helpers ------

	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "InstructMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};