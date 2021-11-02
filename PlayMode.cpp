#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>

GLuint living_room_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > living_room_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("CALLING LIVING_ROOM_MESHES\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("living_room.pnct"), data_path("living_room.boundbox"));
	living_room_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

// angle in radians between two 3d vectors
float angleBetween(glm::vec3 x, glm::vec3 y) {
    return glm::acos(glm::dot(glm::normalize(x), glm::normalize(y)));
}

Load< Scene > living_room_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("living_room.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = living_room_meshes->lookup(mesh_name);
        // BoundBox const &bbox = living_room_meshes->lookup_bound_box(mesh_name);
        // // std::cout << mesh_name << " "<< transform->top_stand << " " << transform->bot_stand 
        // //                       << " " << transform->front_stand << " " << transform->back_stand
        // //                       << " " << transform->left_stand << " " << transform->right_stand << std::endl;

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

        // drawable.transform->bbox[0] = bbox.P1;
        // drawable.transform->bbox[1] = bbox.P2;
        // drawable.transform->bbox[2] = bbox.P3;
        // drawable.transform->bbox[3] = bbox.P4;
        // drawable.transform->bbox[4] = bbox.P5;
        // drawable.transform->bbox[5] = bbox.P6;
        // drawable.transform->bbox[6] = bbox.P7;
        // drawable.transform->bbox[7] = bbox.P8;

        // glm::vec3 up = glm::vec3(0.f, 0.f, 1.f);
        // float deg_45_in_rad = 0.7854f;

        // // std::cout << bbox.P1[0] << " " << bbox.P1[1] << " " << bbox.P1[2] << std::endl;
        // drawable.transform->top_n = glm::cross(bbox.P3 - bbox.P7, bbox.P6 - bbox.P7);
        // drawable.transform->bot_n = glm::cross(bbox.P5 - bbox.P8, bbox.P4 - bbox.P8);
        // drawable.transform->top_stand = (angleBetween(up, drawable.transform->top_n) < deg_45_in_rad);
        // drawable.transform->bot_stand = (angleBetween(up, drawable.transform->bot_n) < deg_45_in_rad);

        // drawable.transform->front_n = glm::cross(bbox.P3 - bbox.P4, bbox.P8 - bbox.P4);
        // drawable.transform->back_n = glm::cross(bbox.P2 - bbox.P6, bbox.P5 - bbox.P6);
        // drawable.transform->front_stand = (angleBetween(up, drawable.transform->front_n) < deg_45_in_rad);
        // drawable.transform->back_stand = (angleBetween(up, drawable.transform->back_n) < deg_45_in_rad);

        // drawable.transform->left_n = glm::cross(bbox.P6 - bbox.P7, bbox.P8 - bbox.P7);
        // drawable.transform->right_n = glm::cross(bbox.P2 - bbox.P1, bbox.P4 - bbox.P1);
        // drawable.transform->left_stand = (angleBetween(up, drawable.transform->left_n) < deg_45_in_rad);
        // drawable.transform->right_stand = (angleBetween(up, drawable.transform->right_n) < deg_45_in_rad);


        // std::cout << mesh_name << " "<< drawable.transform->top_stand << " " << drawable.transform->bot_stand 
        //                       << " " << drawable.transform->front_stand << " " << drawable.transform->back_stand
        //                       << " " << drawable.transform->left_stand << " " << drawable.transform->right_stand << std::endl;


		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = living_room_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

// TODO: find a song sample
Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

void PlayMode::AttachToGround(Scene::Transform *transform) {
    printf("Calling AttachToGround\n");
    printf("1\n");
    player.ground = transform;
    if (transform->top_stand) {
        printf("2\n");
        player.surface = TOP;
        printf("3\n");
        player.transform->position.z = transform->bbox[2].z+ 1.0f;
        printf("4\n");
    } else if (transform->bot_stand) {
        player.surface = BOT;
        player.transform->position.z = transform->bbox[3].z+ 1.0f;
    } else if (transform->front_stand) {
        player.surface = FRONT;
        player.transform->position.z = transform->bbox[3].z+ 1.0f;
    } else if (transform->back_stand) {
        player.surface = BACK;
        player.transform->position.z = transform->bbox[0].z+ 1.0f;
    } else if (transform->left_stand) {
        player.surface = LEFT;
        player.transform->position.z = transform->bbox[7].z+ 1.0f;
    } else if (transform->right_stand) {
        player.surface = RIGHT;
        player.transform->position.z = transform->bbox[2].z+ 1.0f;
    } else {
        std::cout << transform->name << " does not have a standable surface " << std::endl;
    }

    player.ground_level = player.transform->position.z;
}

PlayMode::PlayMode() : scene(*living_room_scene) {

    // get player transform - TODO don't loop through again
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Player") {
            player.transform = drawable.transform;
        }
    }

    for (auto &drawable : scene.drawables) {
        BoundBox const &bbox = living_room_meshes->lookup_bound_box(drawable.transform->name);
        // std::cout << mesh_name << " "<< transform->top_stand << " " << transform->bot_stand 
        //                       << " " << transform->front_stand << " " << transform->back_stand
        //                       << " " << transform->left_stand << " " << transform->right_stand << std::endl;

        drawable.transform->bbox[0] = bbox.P1;
        drawable.transform->bbox[1] = bbox.P2;
        drawable.transform->bbox[2] = bbox.P3;
        drawable.transform->bbox[3] = bbox.P4;
        drawable.transform->bbox[4] = bbox.P5;
        drawable.transform->bbox[5] = bbox.P6;
        drawable.transform->bbox[6] = bbox.P7;
        drawable.transform->bbox[7] = bbox.P8;

        glm::vec3 up = glm::vec3(0.f, 0.f, 1.f);
        float deg_45_in_rad = 0.7854f;

        // std::cout << bbox.P1[0] << " " << bbox.P1[1] << " " << bbox.P1[2] << std::endl;
        drawable.transform->top_n = glm::cross(bbox.P3 - bbox.P7, bbox.P6 - bbox.P7);
        drawable.transform->bot_n = glm::cross(bbox.P5 - bbox.P8, bbox.P4 - bbox.P8);
        drawable.transform->top_stand = (angleBetween(up, drawable.transform->top_n) < deg_45_in_rad);
        drawable.transform->bot_stand = (angleBetween(up, drawable.transform->bot_n) < deg_45_in_rad);

        drawable.transform->front_n = glm::cross(bbox.P3 - bbox.P4, bbox.P8 - bbox.P4);
        drawable.transform->back_n = glm::cross(bbox.P2 - bbox.P6, bbox.P5 - bbox.P6);
        drawable.transform->front_stand = (angleBetween(up, drawable.transform->front_n) < deg_45_in_rad);
        drawable.transform->back_stand = (angleBetween(up, drawable.transform->back_n) < deg_45_in_rad);

        drawable.transform->left_n = glm::cross(bbox.P6 - bbox.P7, bbox.P8 - bbox.P7);
        drawable.transform->right_n = glm::cross(bbox.P2 - bbox.P1, bbox.P4 - bbox.P1);
        drawable.transform->left_stand = (angleBetween(up, drawable.transform->left_n) < deg_45_in_rad);
        drawable.transform->right_stand = (angleBetween(up, drawable.transform->right_n) < deg_45_in_rad);



        auto transform = drawable.transform;
        std::cout << transform->name << " "<< transform->top_stand << " " << transform->bot_stand 
                            << " " << transform->front_stand << " " << transform->back_stand
                            << " " << transform->left_stand << " " << transform->right_stand << std::endl;
        std::cout << transform->name << " "<< transform->bbox[3].x << " " << transform->bbox[3].y 
                            << " " << transform->bbox[3].z << std::endl;
        if (drawable.transform->name == "Facing") {
            player.facing = drawable.transform;
        } else if (drawable.transform->name == "Floor") {
            player.ground = drawable.transform;
            AttachToGround(drawable.transform);
        }
	}

    if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	player.camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
            space.downs += 1;
            space.pressed = true;
            return true;
        }
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
            space.pressed = false;
            return true;
        }
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);

            // face left, face right
			glm::vec3 up = glm::vec3(0.f, 0.f, 1.f);
			player.transform->rotation = glm::angleAxis(-motion.x * player.camera->fovy, up) * player.transform->rotation;

            // face up, face down [TODO: let camera rotate up and down]
			// float pitch = glm::pitch(player.camera->transform->rotation);
			// pitch += motion.y * player.camera->fovy;
			//camera looks down -z (basically at the player's feet) when pitch is at zero.
			// pitch = std::min(pitch, 0.95f * 3.1415926f);
			// pitch = std::max(pitch, 0.05f * 3.1415926f);
			// player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//move player:
	{
        

		//combine inputs into a move:
		constexpr float ground_speed = 8.0f;
        constexpr float air_speed = 5.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.y = -1.0f;
		if (!left.pressed && right.pressed) move.y = 1.0f;
		if (down.pressed && !up.pressed) move.x = 1.0f;
		if (!down.pressed && up.pressed) move.x = -1.0f;
        if (!player.jumping && space.pressed)  {
            player.jumping = true;
        }

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) {
            if (player.jumping) {
                move = glm::normalize(move) * air_speed * elapsed;
            } else {
                move = glm::normalize(move) * ground_speed * elapsed;
            }
            glm::vec3 movement = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.f, 1.f) - player.transform->position;
		    player.transform->position += movement;
        }

        if (player.jumping) {
            player.air_time += elapsed;
            float player_starting_height = player.ground_level + 1.0f;
            float height = player_starting_height + player.init_up_v * player.air_time + 0.5f * gravity * player.air_time * player.air_time;
            if (height <= player_starting_height) {
                player.transform->position.z = player_starting_height;
                player.jumping = false;
                player.air_time = 0.0f;
            } else {
                player.transform->position.z = height;
            }
        }
        
	}

    // // SOURCE: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
    // // TODO: Don't do aabb collision - maybe represent player as a set of points?
    // auto intersect[this](glm::vec3 min1, glm::vec3 max1, glm::vec3 min2, glm::vec3 max2) = {
    //     return  (min1.x <= max2.x && max1.x >= min2.x) &&
    //             (min1.y <= max2.y && max1.y >= min2.y) &&
    //             (min1.z <= max2.z && max1.z >= min2.z);
    // };

    // // check for collisions:
    // {
    //     float p_min_x = std::min(player.transform.P1.x, player.transform);
    //     float p_min_y = ;
    //     float p_min_z = ;
    //     glm::vec3 player_min = glm::();
    //     glm::vec3 player_max = glm::();

    //     for (auto &drawable : scene.drawables) {
    //         if (drawable.transform->name != "Player") {
    //             drawable.transform->
    //             std::max();
    //         }
    //     }
    // }

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
    space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*player.camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
