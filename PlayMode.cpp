#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <iostream>

GLuint living_room_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > living_room_meshes(LoadTagDefault, []() -> MeshBuffer const * {
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

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

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

PlayMode::PlayMode() : scene(*living_room_scene) {
    // Save target heights for falling objects
    float rug_height = 0.0f;
    float sidetable_x_min = 0, sidetable_x_max = 0, sidetable_y_min = 0, sidetable_y_max = 0;

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

        {
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
        }
    
        // auto transform = drawable.transform;
        // std::cout << transform->name << " "<< transform->top_stand << " " << transform->bot_stand 
        //                     << " " << transform->front_stand << " " << transform->back_stand
        //                     << " " << transform->left_stand << " " << transform->right_stand << std::endl;
        // std::cout << transform->name << " "<< transform->bbox[3].x << " " << transform->bbox[3].y 
        //                     << " " << transform->bbox[3].z << std::endl;
        if (drawable.transform->name == "Facing") {
            player.facing = drawable.transform;
        } else if (drawable.transform->name == "Player") {
            player.transform = drawable.transform;
            player.tip = player.transform->position;
            player.tip.z += 1.0f;
            player.base = player.transform->position;
            player.base.z -= 1.0f;
            player.starting_height = player.transform->position.z;
        } 

        // ############### TODO: MOVE INITIALIZATION CONSTRUCTOR TO ROOMOBJECT CONSTRUCTOR
        if (drawable.transform->name != "Player") {
            
            // ###################################################### 
            // For collection COLLISION objects, set CollisionType 
            // ######################################################
            CollisionType type = CollisionType::None;            
            if (drawable.transform->name == "Vase")      type = CollisionType::PushOff;
            else if (drawable.transform->name == "Key")  type = CollisionType::Swat;
            else if (drawable.transform->name == "Mug")  type = CollisionType::KnockOver;

            objects.push_back( RoomObject(drawable.transform, type) );

            if (drawable.transform->name == "Rug") rug_height = objects.back().capsule.tip.z;
            if (drawable.transform->name == "SideTable") {
                sidetable_x_min = drawable.transform->bbox[1].x;
                sidetable_x_max = drawable.transform->bbox[5].x;
                sidetable_y_min = drawable.transform->bbox[1].y;
                sidetable_y_max = drawable.transform->bbox[2].y;
            }
        }
	}

    // ############################################################################ 
    // TODO: Search for list of FALLING objects to set start start_height, end_height 
    // ############################################################################

    auto vase_iter = find_if(objects.begin(), objects.end(),
                            [](const RoomObject &elem) { return elem.transform->name == "Vase"; });
    RoomObject &vase_obj = *(vase_iter);
    vase_obj.start_height = vase_obj.transform->position.z;
    vase_obj.end_height   = rug_height;
    vase_obj.x_min = sidetable_x_min; vase_obj.x_max = sidetable_x_max;
    vase_obj.y_min = sidetable_y_min; vase_obj.y_max = sidetable_y_max;



    {
        auto player_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [](const Scene::Drawable & elem) { return elem.transform->name == "Player"; });
        scene.drawables.erase(player_iter);

        auto wall1_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                    [](const Scene::Drawable & elem) { return elem.transform->name == "Wall1"; });
        wall1 = (*wall1_iter).transform;
        scene.drawables.erase(wall1_iter);

        auto wall2_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                    [](const Scene::Drawable & elem) { return elem.transform->name == "Wall2"; });
        wall2 = (*wall2_iter).transform;
        scene.drawables.erase(wall2_iter);

        auto wall3_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                    [](const Scene::Drawable & elem) { return elem.transform->name == "Wall3"; });
        wall3 = (*wall3_iter).transform;
        scene.drawables.erase(wall3_iter);

        auto wall4_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                    [](const Scene::Drawable & elem) { return elem.transform->name == "Wall4"; });
        wall4 = (*wall4_iter).transform;
        scene.drawables.erase(wall4_iter);
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
        } else if (evt.key.keysym.sym == SDLK_g) {
            swat.downs += 1;
            swat.pressed = true;
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
        } else if (evt.key.keysym.sym == SDLK_g) {
            swat.pressed = false;
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
			// // camera looks down -z (basically at the player's feet) when pitch is at zero.
			// pitch = std::min(pitch, 0.95f * 3.1415926f);
			// pitch = std::max(pitch, 0.05f * 3.1415926f);
			// player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

			return true;
		}
	}

	return false;
}

std::string PlayMode::capsule_collide(RoomObject &current_obj, glm::vec3 *pen_normal, float *pen_depth) {
    for (auto obj : objects) {
        if (obj.name == "Rug") continue; // Rug is kinda blocky
        if (obj.name == "Player") continue;
        if (obj.name == "Facing") continue;
        if (obj.name == "CatPlayer") continue;

        // TODO uncomment this later, floor collisions are inevitable
        // Set height to floor level when floor collision is detected?
        if (obj.name == "Floor") continue;      
        if (obj.name == current_obj.name) continue;

        auto capsule = current_obj.capsule;

        // ---------------------< START HEERE - SAVE + GETN INFO ABOUT OCLLISION MAG+DIR

        SurfaceType surface;
        if (capsule_bbox_collision(capsule.tip, capsule.base, capsule.radius, obj.transform->bbox, &surface, pen_normal, pen_depth)) {
            std::cout << "\n!!! CAPSULE COLLISION: " << obj.name << ", direction: " 
            << glm::to_string(*pen_normal) << ", depth: " << *pen_depth << std::endl;
            return obj.name;
        }
    }
    return "";
}


// TODO extend to save penetration normal, depth to compute sliding for player
// std::string PlayMode::collide(RoomObject *collided_object) {
std::string PlayMode::collide() {
    SurfaceType surface;
    glm::vec3 penetration_normal;
    float penetration_depth;

    for (auto obj : objects) {
        if (obj.name == "Rug") continue; // Rug is kinda blocky
        if (obj.name == "Player") continue;
        if (obj.name == "Facing") continue;
        if (obj.name == "CatPlayer") continue;

        if (capsule_bbox_collision(player.tip, player.base, player.radius, obj.transform->bbox, &surface, &penetration_normal, &penetration_depth)) {
            // *collided_object = obj;
            return obj.transform->name;
        }
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall1->bbox, &surface, &penetration_normal, &penetration_depth)) {
        return wall1->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall2->bbox, &surface, &penetration_normal, &penetration_depth)) {
        return wall2->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall3->bbox, &surface, &penetration_normal, &penetration_depth)) {
        return wall3->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall4->bbox,&surface, &penetration_normal, &penetration_depth)) {
        return wall4->name;
    }

    return "";
}



void PlayMode::update(float elapsed) {
    glm::vec3 prev_player_position = player.transform->position;

	//move player:
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
    if (!player.swatting && swat.pressed) {
        player.swatting = true;
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
        player.tip = player.transform->position;
        player.tip.z += 1.0f;
        player.base = player.transform->position;
        player.base.z -= 1.0f;
    }

    // #####################################################################
    //                          Handle Collisions
    // #####################################################################

    // RoomObject collision_obj;
    // std::string object_collide_name = collide(&collision_obj);
    std::string object_collide_name = collide();

    auto collision_obj_iter = find_if(objects.begin(), objects.end(),
                                        [object_collide_name](const RoomObject &elem) { return elem.name == object_collide_name; });
    RoomObject &collision_obj = *(collision_obj_iter);

    if (player.swatting && collision_obj.collision_type == CollisionType::Swat && !collision_obj.done) {
        auto col_drawable_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == object_collide_name; });
        
        if (col_drawable_iter != scene.drawables.end()) {
            scene.drawables.erase(col_drawable_iter);
        }
        else {
            std::cout << object_collide_name << " does not exist as a drawable" << std::endl;
        }
        collision_obj.done = true;

        // Also erase object itself
        objects.erase(collision_obj_iter);

        score += 3;

        player.swatting = false;
    } 
    else if (collision_obj.collision_type == CollisionType::PushOff && !collision_obj.done) {
        // Save object's original position
        collision_obj.prev_position = collision_obj.transform->position;

        // Calculate player's displacement in this timestep
        glm::vec3 offset = (player.transform->position - prev_player_position);
        collision_obj.transform->position += offset;
        for (auto i = 0; i < 8; i++) {
            collision_obj.orig_bbox[i] = collision_obj.transform->bbox[i]; // Save original BBOX position
            collision_obj.transform->bbox[i] += offset;                    // Update to new
        }

        // ############## Test for collisions against other objects ##############
        if (capsule_collide(collision_obj, &collision_obj.pen_dir, &collision_obj.pen_depth) == "") {    // Move capsule in absence of collisions
            collision_obj.capsule.tip  = collision_obj.capsule.tip  + (collision_obj.transform->position - collision_obj.prev_position);
            collision_obj.capsule.base = collision_obj.capsule.base + (collision_obj.transform->position - collision_obj.prev_position);

            if (!collision_obj.collided && 
                !((collision_obj.x_min <= collision_obj.transform->bbox[5].x && collision_obj.transform->bbox[1].x <= collision_obj.x_max)
                && (collision_obj.y_min <= collision_obj.transform->bbox[2].y && collision_obj.transform->bbox[1].y <= collision_obj.y_max))) {
                collision_obj.is_falling = true;
            }
        }
        else {  // ------- COLLISION OCCURED -------
            // Slide-alone code inspired from https://wickedengine.net/2020/04/26/capsule-collision-detection/, "Usage"
            // TODO: Assumes objects are off the same weight - can add per-object weights (i.e. heavier objects = slower velocities)
            glm::vec3 obj_velocity = offset / elapsed; //(collision_obj.transform->position - collision_obj.prev_position) / elapsed;
            float obj_velocity_length = glm::length(obj_velocity);

            obj_velocity = glm::normalize(obj_velocity);

            if (!glm::isnan(obj_velocity.x) && !glm::isnan(obj_velocity.y) && !glm::isnan(obj_velocity.z)) {

                // Fix velocity
                glm::vec3 undesired_motion = collision_obj.pen_dir * glm::dot(collision_obj.pen_dir, obj_velocity);
                glm::vec3 desired_motion = obj_velocity - undesired_motion;
                obj_velocity = obj_velocity_length * desired_motion;

                // Re-do object movement with displacement vector calculated using this new velocity
                glm::vec3 displacement = obj_velocity * elapsed;
                collision_obj.transform->position = collision_obj.prev_position + displacement;
                for (auto i = 0; i < 8; i++) {
                    collision_obj.transform->bbox[i] = collision_obj.orig_bbox[i] + displacement;
                }
                collision_obj.capsule.tip  += displacement;
                collision_obj.capsule.base += displacement;
            }
        }
    }
    else if (object_collide_name != "") { // undo movement
        player.transform->position = prev_player_position;
        player.tip = prev_player_position;
        player.tip.z += 1.0f;
        player.base = prev_player_position;
        player.base.z -= 1.0f;
    }

    // ##################### Resolve remaining collision behavior #####################
    for (auto &obj : objects) {
        if (obj.collision_type == CollisionType::PushOff) {
            if (!obj.done && obj.is_falling) {

                obj.air_time += elapsed;

                float height = obj.start_height + 0.5f * gravity * obj.air_time * obj.air_time;
                if (height <= obj.end_height) {

                    obj.transform->position.z = obj.end_height;
                    obj.capsule.base.z = obj.end_height;
                    obj.capsule.tip.z = obj.end_height + obj.capsule.height;

                    obj.is_falling = false;
                    obj.air_time = 0.0f;

                    obj.transform->bbox[5].z = obj.end_height + obj.capsule.height;
                    obj.transform->bbox[1].z = obj.end_height + obj.capsule.height;
                    obj.transform->bbox[2].z = obj.end_height + obj.capsule.height;
                    obj.transform->bbox[6].z = obj.end_height + obj.capsule.height;
                    obj.transform->bbox[0].z = obj.end_height;
                    obj.transform->bbox[3].z = obj.end_height;
                    obj.transform->bbox[4].z = obj.end_height;
                    obj.transform->bbox[7].z = obj.end_height;

                    score += 5;
                    obj.collided = true;  // prevents user from gaining more points
                    obj.done = true;
                }
                else {
                    obj.transform->position.z = height;
                    obj.capsule.base.z = height;
                    obj.capsule.tip.z = height + obj.capsule.height;

                    obj.transform->bbox[5].z = height + obj.capsule.height;
                    obj.transform->bbox[1].z = height + obj.capsule.height;
                    obj.transform->bbox[2].z = height + obj.capsule.height;
                    obj.transform->bbox[6].z = height + obj.capsule.height;
                    obj.transform->bbox[0].z = height;
                    obj.transform->bbox[3].z = height;
                    obj.transform->bbox[4].z = height;
                    obj.transform->bbox[7].z = height;
                }
            }
        }
    }

    // ######################### Resolve falling player #########################

    prev_player_position = player.transform->position;

    player.air_time += elapsed;
    if (player.jumping) { // jumping
        player.transform->position.z = player.starting_height + player.init_up_v * player.air_time + 0.5f * gravity * player.air_time * player.air_time;
    } else { // just gravity
        player.transform->position.z = player.starting_height + 0.5f * gravity * player.air_time * player.air_time;
    }

    player.tip = player.transform->position;
    player.tip.z += 1.0f;
    player.base = player.transform->position;
    player.base.z -= 1.0f;

    // object_collide_name = collide(&collision_obj);
    object_collide_name = collide();
    if (object_collide_name != "") {
        player.transform->position = prev_player_position;
        player.tip = prev_player_position;
        player.tip.z += 1.0f;
        player.base = prev_player_position;
        player.base.z -= 1.0f;
        player.jumping = false;
        player.air_time = 0.f;
        player.starting_height = player.transform->position.z;
    }

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
    space.downs = 0;
    swat.downs = 0;
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

    { // DISPLAY BOUNDING BOXES FOR DEBUG PURPOSES!!!!!
        glDisable(GL_DEPTH_TEST);
        DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));

        // for (auto obj : objects) {
            // if (obj.name != "Key") continue;
            auto vase_obj_iter = find_if(objects.begin(), objects.end(),
                                            [](const RoomObject & elem) { return elem.name == "Key"; });
            auto obj = *(vase_obj_iter);
            auto tip = obj.capsule.tip;
            auto base = obj.capsule.base;
            auto radius = obj.capsule.radius;

            // tip
            auto A = glm::vec3(tip.x + radius, tip.y + radius, tip.z);
            auto B = glm::vec3(tip.x - radius, tip.y - radius, tip.z);
            auto C = glm::vec3(tip.x + radius, tip.y - radius, tip.z);
            auto D = glm::vec3(tip.x - radius, tip.y + radius, tip.z);
            
            draw_lines.draw(A, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(B, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(D, B, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(A, D, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

            // base
            auto E = glm::vec3(base.x + radius, base.y + radius, base.z);
            auto F = glm::vec3(base.x - radius, base.y - radius, base.z);
            auto G = glm::vec3(base.x + radius, base.y - radius, base.z);
            auto H = glm::vec3(base.x - radius, base.y + radius, base.z);
            
            draw_lines.draw(E, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(F, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(H, F, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(E, H, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

            // sides
            draw_lines.draw(A,E, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            draw_lines.draw(B,F, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            draw_lines.draw(C,G, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            draw_lines.draw(D,H, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
        // }

        for (auto &drawable : scene.drawables) {
            if (drawable.transform->name != "Key") continue;

            
            // draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            // draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

            // draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0xff, 0xff, 0xff));


            // // top
            draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

            // // bottom
            draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

            // left
            draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

            // right
            draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[0], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

            // front
            draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

            // back
            draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

        }

    }

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

        constexpr float score_H = 0.12f;
        lines.draw_text("Score: " + std::to_string(score),
            glm::vec3(-aspect + 0.1f * score_H, -1.0 + 0.1f * score_H + 0.2f, 0.0),
			glm::vec3(score_H, 0.0f, 0.0f), glm::vec3(0.0f, score_H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		ofs = 2.0f / drawable_size.y;
		lines.draw_text("Score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * score_H + ofs, -1.0 + + 0.1f * score_H + ofs + 0.2f, 0.0),
			glm::vec3(score_H, 0.0f, 0.0f), glm::vec3(0.0f, score_H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
