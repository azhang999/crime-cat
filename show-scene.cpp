#include "Mode.hpp"
#include "ShowSceneMode.hpp"
#include "Load.hpp"
#include "GL.hpp"
#include "load_save_png.hpp"
#include "ShowSceneProgram.hpp"

#include <SDL.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <algorithm>

int main(int argc, char **argv) {
#ifdef _WIN32
	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		"scene viewer",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 800,
		SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE //uncomment to allow resizing
		| SDL_WINDOW_ALLOW_HIGHDPI //uncomment for full resolution on high-DPI screens
	);

	//prevent exceedingly tiny windows when resizing:
	SDL_SetWindowMinimumSize(window, 100, 100);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	//On windows, load OpenGL entrypoints: (does nothing on other platforms)
	init_GL();

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//------------ load resources --------------
	call_load_functions();

	//------------ create game mode + make current --------------
	bool usage = false;
	std::string scene_file;
	std::string meshes_file;
    std::string bbox_file;
	if (argc == 2) {
		scene_file = argv[1];
	} else if (argc == 4) {
		scene_file = argv[1];
		meshes_file = argv[2];
        bbox_file = argv[3];
	} else {
		usage = true;
	}
	MeshBuffer *buffer = nullptr;
	GLuint buffer_vao = 0;
	if (meshes_file != "") {
		try {
			buffer = new MeshBuffer(meshes_file, bbox_file);
			buffer_vao = buffer->make_vao_for_program(show_scene_program->program);
		} catch (std::exception &e) {
			std::cerr << "ERROR loading mesh buffer '" << meshes_file << "': " << e.what() << std::endl;
			usage = true;
			buffer = nullptr;
		}
	}
	Scene *scene = nullptr;
	if (scene_file != "") {
		try {
			scene = new Scene();
			scene->load(scene_file, [&buffer,&buffer_vao](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
				if (!buffer_vao) return;
				Mesh const &mesh = buffer->lookup(mesh_name);

				scene.drawables.emplace_back(transform);
				Scene::Drawable &drawable = scene.drawables.back();

				drawable.pipeline = show_scene_program_pipeline;

				drawable.pipeline.vao = buffer_vao;
				drawable.pipeline.type = mesh.type;
				drawable.pipeline.start = mesh.start;
				drawable.pipeline.count = mesh.count;

			});
		} catch (std::exception &e) {
			std::cerr << "ERROR loading scene '" << scene_file << "': " << e.what() << std::endl;
			usage = true;
			scene = nullptr;
		}
	}
	if (!scene) {
		usage = true;
	}
	if (usage) {
		std::cerr << "Usage:\n\t" << argv[0] << " <path/to/scene.scene> [path/to/meshes.pnct]" << std::endl;
		return 1;
	}
	std::cout << "Showing scene from '" << scene_file << "' with";
	if (meshes_file != "") {
		std::cout << " meshes from '" << meshes_file << "'" << std::endl;
	} else {
		std::cout << " no meshes -- consider passing a '.pnct' file as the second argument." << std::endl;
	}
	Mode::set_current(std::make_shared< ShowSceneMode >(*scene));

	//------------ main loop ------------

	//this inline function will be called whenever the window is resized,
	// and will update the window_size and drawable_size variables:
	glm::uvec2 window_size; //size of window (layout pixels)
	glm::uvec2 drawable_size; //size of drawable (physical pixels)
	//On non-highDPI displays, window_size will always equal drawable_size.
	auto on_resize = [&](){
		int w,h;
		SDL_GetWindowSize(window, &w, &h);
		window_size = glm::uvec2(w, h);
		SDL_GL_GetDrawableSize(window, &w, &h);
		drawable_size = glm::uvec2(w, h);
		glViewport(0, 0, drawable_size.x, drawable_size.y);
	};
	on_resize();

	//This will loop until the current mode is set to null:
	while (Mode::current) {
		//every pass through the game loop creates one frame of output
		//  by performing three steps:

		{ //(1) process any events that are pending
			static SDL_Event evt;
			while (SDL_PollEvent(&evt) == 1) {
				//handle resizing:
				if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					on_resize();
				}
				//handle input:
				if (Mode::current && Mode::current->handle_event(evt, window_size)) {
					// mode handled it; great
				} else if (evt.type == SDL_QUIT) {
					Mode::set_current(nullptr);
					break;
				} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_PRINTSCREEN) {
					// --- screenshot key ---
					std::string filename = "screenshot.png";
					std::cout << "Saving screenshot to '" << filename << "'." << std::endl;
					glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
					glReadBuffer(GL_FRONT);
					int w,h;
					SDL_GL_GetDrawableSize(window, &w, &h);
					std::vector< glm::u8vec4 > data(w*h);
					glReadPixels(0,0,w,h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
					for (auto &px : data) {
						px.a = 0xff;
					}
					save_png(filename, glm::uvec2(w,h), data.data(), LowerLeftOrigin);
				}
			}
			if (!Mode::current) break;
		}

		{ //(2) call the current mode's "update" function to deal with elapsed time:
			auto current_time = std::chrono::high_resolution_clock::now();
			static auto previous_time = current_time;
			float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
			previous_time = current_time;

			//if frames are taking a very long time to process,
			//lag to avoid spiral of death:
			elapsed = std::min(0.1f, elapsed);

			Mode::current->update(elapsed);
			if (!Mode::current) break;
		}

		{ //(3) call the current mode's "draw" function to produce output:
		
			Mode::current->draw(drawable_size);
		}

		//Wait until the recently-drawn frame is shown before doing it all again:
		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------
	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;

#ifdef _WIN32
	} catch (std::exception const &e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}
