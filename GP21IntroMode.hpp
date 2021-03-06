#pragma once

/*
 * GP21 intro animation / splash screen.
 *
 */

#include "Mode.hpp"
#include "Sound.hpp"

#include "GL.hpp"

#include <memory>
#include <functional>

struct GP21IntroMode : Mode {
	// Passes a function to be run when this mode is finished/skipped
	// Recommended by Jim McCann, as a modification of https://raw.githubusercontent.com/15-466/15-466-f21-intro/main/GP21IntroMode.hpp
	GP21IntroMode(std::function< void() > on_finish);
	virtual ~GP21IntroMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//will Mode::set_current(next_mode) when done animating:
	// std::shared_ptr< Mode > next_mode;

	//will start playing music on launch, will silence music on quit:
	std::shared_ptr< Sound::PlayingSample > music;

	//will draw a fancy set of cubes with dynamically generated vertices:
	GLuint color_program = 0;
	GLuint OBJECT_TO_CLIP_mat4 = -1U;

	GLuint vertex_buffer = 0;
	GLuint vertex_buffer_for_color_program = 0;

	struct Vertex {
		Vertex(glm::vec2 const &Position_, glm::u8vec4 Color_) : Position(Position_), Color(Color_) { }
		glm::vec2 Position;
		glm::u8vec4 Color;
	};

	struct Cube {
		glm::ivec3 target;
		//for falling animation:
		float at = 0.0f;
		float velocity = 0.0f;
		//[0,1] key used to trigger animation starts:
		float key = 0.0f;
	};
	std::vector< Cube > cubes;

	//animation is timed based on this accumulator:
	float time = 0.0f;

	// Routine to initialize next mode
	std::function< void()> goto_nextmode;
};