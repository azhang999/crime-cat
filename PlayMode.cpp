#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <iostream>

GLuint cat_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > cat_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating Cat Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("cat.pnct"), data_path("cat.boundbox"));
	cat_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint living_room_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > living_room_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("living_room.pnct"), data_path("living_room.boundbox"));
	living_room_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint kitchen_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > kitchen_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating Kitchen Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("kitchen.pnct"), data_path("kitchen.boundbox"));
	kitchen_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

// angle in radians between two 3d vectors
float angleBetween(glm::vec3 x, glm::vec3 y) {
    return glm::acos(glm::dot(glm::normalize(x), glm::normalize(y)));
}

Load< Scene > cat_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("cat.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = cat_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = cat_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Scene > living_room_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("living_room.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
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

Load< Scene > kitchen_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("kitchen.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
		Mesh const &mesh = kitchen_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = kitchen_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

// TODO: find a song sample
Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

float get_top_height(Scene::Transform *transform) {
    if (transform->top_stand) {
        return transform->bbox[2].z;
    } else if (transform->bot_stand) {
        return transform->bbox[4].z;
    } else if (transform->back_stand) {
        return transform->bbox[1].z;
    } else if (transform->front_stand) {
        return transform->bbox[2].z;
    } else if (transform->left_stand) {
        return transform->bbox[5].z;
    } else if (transform->right_stand) {
        return transform->bbox[1].z;
    } else {
        throw std::runtime_error(transform->name + " does not have a standable surface\n");
    }
    return 0.f;
}

void PlayMode::GenerateBBox(Scene &scene, Load<MeshBuffer> &meshes) {

    for (auto &drawable : scene.drawables) {
        BoundBox const &bbox = meshes->lookup_bound_box(drawable.transform->name);

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

        // auto transform = drawable.transform;
        // std::cout << transform->name << " "<< transform->top_stand << " " << transform->bot_stand 
        //                     << " " << transform->front_stand << " " << transform->back_stand
        //                     << " " << transform->left_stand << " " << transform->right_stand << std::endl;
        // std::cout << transform->name << " "<< transform->bbox[3].x << " " << transform->bbox[3].y 
        //                     << " " << transform->bbox[3].z << std::endl;
        if (drawable.transform->name == "Player") {
            player.transform = drawable.transform;
            player.tip = player.transform->position;
            player.tip.z += 1.0f;
            player.base = player.transform->position;
            player.base.z -= 1.0f;
            player.starting_height = player.transform->position.z;
        } 
	}
}

// animation code
bool SearchFrameByName(Scene &scene, std::string name) {
    auto frame_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [name](const Scene::Drawable & elem) { return elem.transform->name == name; });
    return frame_iter != scene.drawables.end();
}

bool RemoveFrameByName(Scene &scene, std::string name) {
    auto frame_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [name](const Scene::Drawable & elem) { return elem.transform->name == name; });
    if (frame_iter == scene.drawables.end()) return false;
    scene.drawables.erase(frame_iter);
    return true;
}

void AddFrame(Scene &scene, Scene::Drawable &drawable) {
    scene.drawables.push_back(drawable);
}

void RemoveAllFrames(Scene &scene) {
    while (scene.drawables.begin() != scene.drawables.end()) {
        scene.drawables.erase(scene.drawables.begin());
    }
}

void GetFrames(Scene &scene, PlayMode::Animation &animation, std::string name) {
    animation.name = name;
    for (uint32_t idx = 0; idx < animation.frame_times.size(); ++idx) {
        std::string frame_name = animation.name + std::to_string(idx);
        auto frame_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [frame_name](const Scene::Drawable & elem) { return elem.transform->name == frame_name; });
        animation.frames.push_back(*frame_iter);
        scene.drawables.erase(frame_iter);
    }
}

void PlayMode::Animation::animate(Scene &scene, bool enable, float elapsed) {
    if (!enable) return;

    timer += elapsed;
    std::string old_frame_name = name + std::to_string(frame_idx);
    bool found = SearchFrameByName(scene, old_frame_name);

    if (!found || frame_times[frame_idx] <= timer) {
        if (RemoveFrameByName(scene, old_frame_name)) { // continuing this animation
            frame_idx = (frame_idx + 1) % frames.size();
        } else { // was in the middle of another animation
            RemoveAllFrames(scene);
            frame_idx = 0;
        }
        timer = 0.f;
        AddFrame(scene, frames[frame_idx]);
    }
}

void PlayMode::generate_room_objects(Scene &scene, std::vector<RoomObject> &objects, RoomType room_type) {

    if (room_type == RoomType::LivingRoom) {
        float rug_height = 0.0f;
        float sidetable_x_min = 0, sidetable_x_max = 0, sidetable_y_min = 0, sidetable_y_max = 0;

        for (auto &drawable : scene.drawables) {
            if (drawable.transform->name == "Player") continue;
            
            CollisionType type = CollisionType::None;  
            if (drawable.transform->name == "Vase")      type = CollisionType::PushOff;
            else if (drawable.transform->name == "Key")  type = CollisionType::Steal;
            else if (drawable.transform->name == "Mug")  type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Pillow")      type = CollisionType::Destroy;
            else if (drawable.transform->name == "Pillow.001")  type = CollisionType::Destroy;
            else if (drawable.transform->name == "Magazine")        type = CollisionType::KnockOver;

            objects.push_back( RoomObject(drawable.transform, type) );

            if (drawable.transform->name == "Rug") rug_height = objects.back().capsule.tip.z;
            if (drawable.transform->name == "SideTable") {
                sidetable_x_min = drawable.transform->bbox[1].x;
                sidetable_x_max = drawable.transform->bbox[5].x;
                sidetable_y_min = drawable.transform->bbox[1].y;
                sidetable_y_max = drawable.transform->bbox[2].y;
            }
            if (drawable.transform->name == "Floor") {
                living_room_floor = drawable.transform;
            }
        }

        // ----- Search for FALLING objects to set start/end heights -----
        auto vase_iter = find_if(objects.begin(), objects.end(),
                                [](const RoomObject &elem) { return elem.transform->name == "Vase"; });
        RoomObject &vase_obj = *(vase_iter);
        vase_obj.start_height = vase_obj.transform->position.z;
        vase_obj.end_height   = rug_height;
        vase_obj.x_min = sidetable_x_min; vase_obj.x_max = sidetable_x_max;
        vase_obj.y_min = sidetable_y_min; vase_obj.y_max = sidetable_y_max;

        // ----------- Save floor -----------
    }
    else if (room_type == RoomType::Kitchen) {
        for (auto &drawable : scene.drawables) {
            if (drawable.transform->name == "Player") continue;
            
            CollisionType type = CollisionType::None;
            objects.push_back( RoomObject(drawable.transform, type) );

            if (drawable.transform->name == "Floor.001") {
                kitchen_floor = drawable.transform;
            }
            if (drawable.transform->name == "Counter") {
                counter_transform = drawable.transform;
            }
        }
    }
}

void PlayMode::switch_rooms(RoomType room_type) {
    if (room_type == RoomType::LivingRoom) {
        current_room = RoomType::LivingRoom;
        current_scene   = &living_room_scene;
        current_objects = &living_room_objects;
    }
    else if (room_type == RoomType::Kitchen) {
        current_room = RoomType::Kitchen;
        current_scene   = &kitchen_scene;
        current_objects = &kitchen_objects;
    }
}

PlayMode::PlayMode() : 
    cat_scene(*cat_scene_load), living_room_scene(*living_room_scene_load), kitchen_scene(*kitchen_scene_load) {
    
    GenerateBBox(cat_scene, cat_meshes);

    // remove player capsule from being drawn
    RemoveFrameByName(cat_scene, "Player");

    player_walking.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    player_up_jump.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 1000000.f}; // don't want to cycle
    player_down_jump.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 1000000.f}; // don't want to cycle
    player_swat.frame_times = {0.1f, 0.1f, 100000.f}; // don't want to cycle

    GetFrames(cat_scene, player_walking, "Walk");
    GetFrames(cat_scene, player_up_jump, "UpJump");
    GetFrames(cat_scene, player_down_jump, "DownJump");
    GetFrames(cat_scene, player_swat, "Swat");

    // start cat with Walk0 fame
    AddFrame(cat_scene, player_walking.frames[0]);

    if (cat_scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(cat_scene.cameras.size()));
	player.camera = &cat_scene.cameras.front();

    GenerateBBox(living_room_scene, living_room_meshes);
    GenerateBBox(kitchen_scene, kitchen_meshes);

    // ##################################################################
    //          Premature optimization is the root of all evil :)
    // ###############################################################

    generate_room_objects(living_room_scene, living_room_objects, RoomType::LivingRoom);
    generate_room_objects(kitchen_scene, kitchen_objects, RoomType::Kitchen);

    // ----- Start in living room -----
    switch_rooms(RoomType::LivingRoom);
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
            phi -= motion.x;
            theta -= motion.y;
			theta = -std::min(-theta, 0.5f * 3.1415926f);
			theta = -std::max(-theta, 0.05f * 3.1415926f);
			return true;
		}
	}

	return false;
}

SurfaceType get_top_surface_type(Scene::Transform *transform) {
    if (transform->top_stand) {
        return TOP;
    } else if (transform->bot_stand) {
        return BOT;
    } else if (transform->back_stand) {
        return BACK;
    } else if (transform->front_stand) {
        return FRONT;
    } else if (transform->left_stand) {
        return LEFT;
    } else if (transform->right_stand) {
        return RIGHT;
    } else {
        throw std::runtime_error(transform->name + " does not have a standable surface\n");
    }
}

std::string PlayMode::capsule_collide(RoomObject &current_obj, glm::vec3 *pen_normal, float *pen_depth) {
    for (auto obj : *current_objects) {
        if (obj.name == "Rug") continue; // Rug is kinda blocky
        // if (obj.name == "Player") continue;
        // if (obj.name == "Facing") continue;
        // if (obj.name == "CatPlayer") continue;

        // TODO uncomment this later, floor collisions are inevitable
        // Set height to floor level when floor collision is detected?
        if (obj.name == "Floor") continue;      
        if (obj.name == current_obj.name) continue;

        auto capsule = current_obj.capsule;
        SurfaceType surface;
        if (capsule_bbox_collision(capsule.tip, capsule.base, capsule.radius, obj.transform->bbox, &surface, pen_normal, pen_depth)) {
            // std::cout << "\n!!! CAPSULE COLLISION: " << obj.name << ", direction: " 
            // << glm::to_string(*pen_normal) << ", depth: " << *pen_depth << std::endl;

            // if (std::isnan((*pen_normal).x) || std::isnan((*pen_normal).x) || std::isnan((*pen_normal).z)) {
            //     std::cout << "----------- nevermind ignore this it's nan -----------" << std::endl;
            //     return "";
            // }

            return obj.name;
        }
    }
    return "";
}


// TODO extend to save penetration normal, depth to compute sliding for player
// std::string PlayMode::collide(RoomObject *collided_object) {
Scene::Transform *PlayMode::collide() {
    SurfaceType surface;

    switch_rooms(RoomType::LivingRoom);
    for (auto obj : *current_objects) {
        if (obj.name == "Rug") continue; // Rug is kinda blocky
        // if (obj.name == "Player") continue;
        // if (obj.name == "Facing") continue;
        // if (obj.name == "CatPlayer") continue;

        if (capsule_bbox_collision(player.tip, player.base, player.radius, obj.transform->bbox, &surface, &penetration_normal, &penetration_depth)) {
            // *collided_object = obj;
            return obj.transform;
        }
    }

    switch_rooms(RoomType::Kitchen);
    for (auto obj : *current_objects) {
        // if (obj.name == "Rug") continue; // Rug is kinda blocky
        // if (obj.name == "Player") continue;
        // if (obj.name == "Facing") continue;
        // if (obj.name == "CatPlayer") continue;

        if (capsule_bbox_collision(player.tip, player.base, player.radius, obj.transform->bbox, &surface, &penetration_normal, &penetration_depth)) {
            // *collided_object = obj;
            return obj.transform;
        }
    }

    // -------- TODO: remove these after walls are added back into the scene --------

    // if (capsule_bbox_collision(player.tip, player.base, player.radius, counter_transform->bbox, &surface, &penetration_normal, &penetration_depth, get_top_surface_type(obj.transform))) {
    //     return counter_transform;
    // }
    // if (capsule_bbox_collision(player.tip, player.base, player.radius, living_room_floor->bbox, &surface, &penetration_normal, &penetration_depth)) {
    //     if (current_room != RoomType::LivingRoom) switch_rooms(RoomType::LivingRoom);
    //     return living_room_floor;
    // }
    // if (capsule_bbox_collision(player.tip, player.base, player.radius, kitchen_floor->bbox, &surface, &penetration_normal, &penetration_depth)) {
    //     // std::cout << "------ Walked into KITCHEN. Current room = " << ((current_room == RoomType::LivingRoom) ? "LivingRoom" : "Kitchen");
    //     if (current_room != RoomType::Kitchen) switch_rooms(RoomType::Kitchen);
    //     // std::cout << ", NEW ROOM = " << ((current_room == RoomType::LivingRoom) ? "LivingRoom" : "Kitchen") << std::endl;
    //     return kitchen_floor;
    // }

    return nullptr;
}

void PlayMode::update(float elapsed) {
    if (game_over) return;

    game_timer -= elapsed;
    if (game_timer <= 0.f) {
        game_over = true;
        game_timer = 0.f;
    }

    glm::vec3 prev_player_position = player.transform->position;

	//move player:
    //combine inputs into a move:
    constexpr float ground_speed = 8.0f;
    constexpr float air_speed = 5.0f;
    glm::vec2 move = glm::vec2(0.0f);
    // if (left.pressed && !right.pressed) move.y = -1.0f;
    // if (!left.pressed && right.pressed) move.y = 1.0f;

    bool moved = down.pressed || up.pressed || left.pressed || right.pressed;

    if (down.pressed && !up.pressed) move.x = 1.0f;
    if (!down.pressed && up.pressed) move.x = -1.0f;
    if (!player.jumping && space.pressed)  {
        player.jumping = true;
    }
    if (!player.swatting && swat.pressed) {
        player.swatting = true;
    }

    if (player.swatting) {
        player.swatting_timer += elapsed;
        float total_swat_time = 0.3f;
        if (player.swatting_timer >= total_swat_time) {
            player.swatting = false;
            player.swatting_timer = 0.f;
        }
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

    { // rotate player
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

        if (left.pressed && !right.pressed)
            player.transform->rotation *= glm::angleAxis(3.0f * elapsed, up);
        if (!left.pressed && right.pressed)
            player.transform->rotation *= glm::angleAxis(-3.0f * elapsed, up);
    }
    
    // #####################################################################
    //                          Handle Collisions
    // #####################################################################

    auto object_collide = collide();
    std::string object_collide_name = "";
    if (object_collide != nullptr) {
        object_collide_name = object_collide->name;
    }

    auto collision_obj_iter = find_if((*current_objects).begin(), (*current_objects).end(),
                                        [object_collide_name](const RoomObject &elem) { return elem.name == object_collide_name; });
    RoomObject &collision_obj = *(collision_obj_iter);

    // --------- Steal object ---------
    if (player.swatting && collision_obj.collision_type == CollisionType::Steal && !collision_obj.done) {
        score += 3;
        collision_obj.done = true;
        player.swatting = false;
        player.swatting_timer = 0.f;

        // Save current scale
        collision_obj.orig_scale = collision_obj.transform->scale;
        collision_obj.transform->scale = glm::vec3(0);
        // Save current position
        collision_obj.prev_position = collision_obj.transform->position;
        collision_obj.transform->position = glm::vec3(-10000);
        // Save current bounding box
        for (auto i = 0; i < 8; i++) {
            collision_obj.orig_bbox[i] = collision_obj.transform->bbox[i];
            collision_obj.transform->bbox[i] = glm::vec3(-10000);
        }
        // Move capsule tip and base, to be reset later (TODO: write a class helper that does this)
        collision_obj.capsule.tip = glm::vec3(-10000);
        collision_obj.capsule.base = glm::vec3(-10000);
        

        // std::cout << "Scale before: " << glm::to_string(collision_obj.orig_scale) << ", scale after: " << glm::to_string(collision_obj.transform->scale) << std::endl;

        // TODO: debug this if object needs to be deleted
        // Erase references to drawable and its object
        // auto col_drawable_iter = find_if((*current_scene).drawables.begin(), (*current_scene).drawables.end(),
        //                         [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == object_collide_name; });
        // but pretty sure we want to keep this object around for later 
        // if (col_drawable_iter != (*current_scene).drawables.end())
        //     (*current_scene).drawables.erase(col_drawable_iter);
        // (*current_objects).erase(obj_iter);
    }
    // --------- Destroy object ---------
    else if (player.swatting && collision_obj.collision_type == CollisionType::Destroy && !collision_obj.done) {
        // TODO: post-collision mesh-switchout
        score += 5;
        collision_obj.done = true;
        player.swatting = false;
    }
    // --------- Knock over object ---------
    else if (collision_obj.collision_type == CollisionType::KnockOver && !collision_obj.done) {
        // TODO: post-collision mesh-switchout
        score += 3;
        collision_obj.done = true;
    }
    // --------- Push object off of surface ---------
    else if (collision_obj.collision_type == CollisionType::PushOff && !collision_obj.done) {
        // std::cout << "I have no idea why this is happening" << std::endl;
        // Save object's original position
        collision_obj.prev_position = collision_obj.transform->position;

        // Calculate player's displacement in this timestep
        glm::vec3 offset = (player.transform->position - prev_player_position);
        collision_obj.transform->position += offset;
        for (auto i = 0; i < 8; i++) {
            collision_obj.orig_bbox[i] = collision_obj.transform->bbox[i]; // Save original BBOX position
            collision_obj.transform->bbox[i] += offset;                    // Update to new
        }

        // Test for collisions against other objects 
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
    // --------- * No collision occured * ---------
    else if (object_collide_name != "") { // undo movement
        player.transform->position = prev_player_position;
        player.tip = prev_player_position;
        player.tip.z += 1.0f;
        player.base = prev_player_position;
        player.base.z -= 1.0f;
    }

    // if (need_to_erase) {
    //     (*current_objects).erase(collision_obj_iter);
    //     std::cout << "REMAINING OBJECTS: " << std::endl;
    //     for (auto &remaining_obj : *(current_objects)) {
    //         std::cout << "\t" << remaining_obj.name << std::endl;
    //     }
    // }

    // ##################### Resolve remaining collision behavior #####################
    for (auto &obj : *current_objects) {
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

    object_collide = collide();
    object_collide_name = "";
    if (object_collide != nullptr) {
        object_collide_name = object_collide->name;
    }

    if (object_collide_name != "") {
        if (penetration_normal.z < 0) {
            penetration_normal.z *= -1.f;
        }

        penetration_depth = std::abs(penetration_depth);

        bool use_up_vec = is_almost_up_vec(penetration_normal);
        // printf("use_up_vec:%d\n", use_up_vec);
        glm::vec3 new_pos;
        if (use_up_vec) {
            float top_height = get_top_height(object_collide);
            new_pos =  player.transform->position;
            new_pos.z = top_height + 1.00001f;
        } else {
            glm::vec3 offset = ((penetration_depth + 0.000001f) * glm::normalize(penetration_normal));
            new_pos =  player.transform->position + offset;
        }

        // place cat slightly above surface
        // float penetration_depth = player.radius - glm::length(intersection_vec);
        // float top_height = get_top_height(object_collide);
        // auto new_pos =  player.transform->position + ((penetration_depth + 0.000001f) * glm::normalize(intersection_vec));
        // auto new_pos = prev_player_position;
        // new_pos.z = top_height + 1.00001f;
        player.transform->position = new_pos;
        player.tip = new_pos;
        player.tip.z += 1.0f;
        player.base = new_pos;
        player.base.z -= 1.0f;
        player.jumping = false;
        player.air_time = 0.f;
        player.starting_height = player.transform->position.z;
    }

    // animate walking
    if (prev_player_position.z == player.transform->position.z) { // potentially walking
        if (player.swatting) {
            player_swat.animate(cat_scene, true, elapsed);
        } else if (moved) {
            player_walking.animate(cat_scene, true, elapsed);
        } else {
            RemoveAllFrames(cat_scene);
            AddFrame(cat_scene, player_walking.frames[player_walking.frame_idx]);
        }
    } else if (prev_player_position.z < player.transform->position.z) { // up jump
        player_up_jump.animate(cat_scene, true, elapsed);
    } else { // down jump
        player_down_jump.animate(cat_scene, true, elapsed);
    }

    {
        // TODO: remove this once the camera is no longer a child of the cat
        player.camera->transform->parent = nullptr;

        glm::vec3 camera_offset = glm::vec3(
            camera_radius * cos(phi + M_PI/2) * sin(theta),
            camera_radius * sin(phi + M_PI/2) * sin(theta),
            camera_radius * cos(theta));
        
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        player.camera->transform->position = player.transform->position + camera_offset;
        player.camera->transform->rotation = glm::angleAxis(phi, up);
        player.camera->transform->rotation *= glm::angleAxis(-theta, right);
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

	cat_scene.draw(*player.camera);
    living_room_scene.draw(*player.camera);
    kitchen_scene.draw(*player.camera);

    // { // DISPLAY BOUNDING BOXES FOR DEBUG PURPOSES!!!!!
    //     glDisable(GL_DEPTH_TEST);
    //     DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));

    //     // for (auto obj : objects) {
    //         auto vase_obj_iter = find_if((*current_objects).begin(), (*current_objects).end(),
    //                                         [](const RoomObject & elem) { return elem.name == "Key"; });
    //         auto obj = *(vase_obj_iter);
    //         auto tip = obj.capsule.tip;
    //         auto base = obj.capsule.base;
    //         auto radius = obj.capsule.radius;

    //         // tip
    //         auto A = glm::vec3(tip.x + radius, tip.y + radius, tip.z);
    //         auto B = glm::vec3(tip.x - radius, tip.y - radius, tip.z);
    //         auto C = glm::vec3(tip.x + radius, tip.y - radius, tip.z);
    //         auto D = glm::vec3(tip.x - radius, tip.y + radius, tip.z);
            
    //         draw_lines.draw(A, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(B, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(D, B, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(A, D, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

    //         // base
    //         auto E = glm::vec3(base.x + radius, base.y + radius, base.z);
    //         auto F = glm::vec3(base.x - radius, base.y - radius, base.z);
    //         auto G = glm::vec3(base.x + radius, base.y - radius, base.z);
    //         auto H = glm::vec3(base.x - radius, base.y + radius, base.z);
            
    //         draw_lines.draw(E, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(F, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(H, F, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(E, H, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

    //         // sides
    //         draw_lines.draw(A,E, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
    //         draw_lines.draw(B,F, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
    //         draw_lines.draw(C,G, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
    //         draw_lines.draw(D,H, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
    //     // }

    //     for (auto &drawable : (*current_scene).drawables) {
    //         if (drawable.transform->name != "Table.005" && drawable.transform->name != "Key") continue;

    //         // draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         // draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

    //         // draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         // draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0xff, 0xff, 0xff));


    //         // top
    //         draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // // bottom
    //         draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

    //         // left
    //         draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // right
    //         draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[0], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // front
    //         draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // back
    //         draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

    //     }

    // }

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
        std::string message;
        if (game_over) {
            message = "Your Owner Came Back, GAME OVER!";
        } else {
            message = "Time Remaining: " + std::to_string(game_timer/ 60.f) + " Minutes";
        }

		lines.draw_text(message,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(message,
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
