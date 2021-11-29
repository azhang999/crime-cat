#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "BlobShadowTextureProgram.hpp"

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
GLuint living_room_meshes_for_blob_shadow_texture_program = 0;
Load< MeshBuffer > living_room_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("living_room.pnct"), data_path("living_room.boundbox"));
	living_room_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    living_room_meshes_for_blob_shadow_texture_program = ret->make_vao_for_program(blob_shadow_texture_program->program);
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
		Mesh const &mesh = living_room_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();
        
        if (transform->name == "CatShadow") {
            drawable.pipeline = blob_shadow_texture_program_pipeline;
            drawable.pipeline.vao = living_room_meshes_for_blob_shadow_texture_program;
        }
        else {
            drawable.pipeline = lit_color_texture_program_pipeline;
            drawable.pipeline.vao = living_room_meshes_for_lit_color_texture_program;
        }
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

Load< Sound::Sample > bg_music(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("blippy_trance.wav"));
});

Load< Sound::Sample > shattering(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("shattering.wav"));
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

bool shadow_intersect(Scene::Transform *transform, glm::vec3 cat_pos, glm::vec3 &intersect_pt) {
    // Raycast down from player position to object top surface bbox planes
    // Using ray-plane intersection equation from http://15462.courses.cs.cmu.edu/fall2021/lecture/geometricqueries/slide_021

    // Gather true "top" plane normal, offset
    glm::vec3 N;
    float offset, x_min, x_max, y_min, y_max;
    if (transform->top_stand) {
        x_min = std::min({transform->bbox[5].x, transform->bbox[1].x, transform->bbox[2].x, transform->bbox[6].x});
        x_max = std::max({transform->bbox[5].x, transform->bbox[1].x, transform->bbox[2].x, transform->bbox[6].x});
        y_min = std::min({transform->bbox[5].y, transform->bbox[1].y, transform->bbox[2].y, transform->bbox[6].y});
        y_max = std::max({transform->bbox[5].y, transform->bbox[1].y, transform->bbox[2].y, transform->bbox[6].y});

        N = glm::normalize(transform->top_n);
        offset = glm::dot(N, transform->bbox[2]);
    }
    else if (transform->bot_stand) {
        x_min = std::min({transform->bbox[4].x, transform->bbox[0].x, transform->bbox[3].x, transform->bbox[7].x});
        x_max = std::max({transform->bbox[4].x, transform->bbox[0].x, transform->bbox[3].x, transform->bbox[7].x});
        y_min = std::min({transform->bbox[4].y, transform->bbox[0].y, transform->bbox[3].y, transform->bbox[7].y});
        y_max = std::max({transform->bbox[4].y, transform->bbox[0].y, transform->bbox[3].y, transform->bbox[7].y});
        
        N = transform->bot_n;
        offset = glm::dot(N, transform->bbox[4]);
    }
    else if (transform->back_stand) {
        x_min = std::min({transform->bbox[5].x, transform->bbox[1].x, transform->bbox[0].x, transform->bbox[4].x});
        x_max = std::max({transform->bbox[5].x, transform->bbox[1].x, transform->bbox[0].x, transform->bbox[4].x});
        y_min = std::min({transform->bbox[5].y, transform->bbox[1].y, transform->bbox[0].y, transform->bbox[4].y});
        y_max = std::max({transform->bbox[5].y, transform->bbox[1].y, transform->bbox[0].y, transform->bbox[4].y});

        N = transform->back_n;
        offset = glm::dot(N, transform->bbox[1]);
    }
    else if (transform->front_stand) {
        x_min = std::min({transform->bbox[6].x, transform->bbox[2].x, transform->bbox[3].x, transform->bbox[7].x});
        x_max = std::max({transform->bbox[6].x, transform->bbox[2].x, transform->bbox[3].x, transform->bbox[7].x});
        y_min = std::min({transform->bbox[6].y, transform->bbox[2].y, transform->bbox[3].y, transform->bbox[7].y});
        y_max = std::max({transform->bbox[6].y, transform->bbox[2].y, transform->bbox[3].y, transform->bbox[7].y});

        N = transform->front_n;
        offset = glm::dot(N, transform->bbox[2]);
    }
    else if (transform->left_stand) {
        x_min = std::min({transform->bbox[5].x, transform->bbox[6].x, transform->bbox[7].x, transform->bbox[4].x});
        x_max = std::max({transform->bbox[5].x, transform->bbox[6].x, transform->bbox[7].x, transform->bbox[4].x});
        y_min = std::min({transform->bbox[5].y, transform->bbox[6].y, transform->bbox[7].y, transform->bbox[4].y});
        y_max = std::max({transform->bbox[5].y, transform->bbox[6].y, transform->bbox[7].y, transform->bbox[4].y});

        N = transform->left_n;
        offset = glm::dot(N, transform->bbox[5]);
    }
    else if (transform->right_stand) {
        x_min = std::min({transform->bbox[1].x, transform->bbox[2].x, transform->bbox[3].x, transform->bbox[0].x});
        x_max = std::max({transform->bbox[1].x, transform->bbox[2].x, transform->bbox[3].x, transform->bbox[0].x});
        y_min = std::min({transform->bbox[1].y, transform->bbox[2].y, transform->bbox[3].y, transform->bbox[0].y});
        y_max = std::max({transform->bbox[1].y, transform->bbox[2].y, transform->bbox[3].y, transform->bbox[0].y});

        N = transform->right_n;
        offset = glm::dot(N, transform->bbox[1]); 
    }
    else throw std::runtime_error(transform->name + " does not have a standable surface\n");

    if ((cat_pos.x < x_min) || (cat_pos.x > x_max) || (cat_pos.y < y_min) || (cat_pos.y > y_max)) return false;


    glm::vec3 d = glm::vec3(0, 0, -cat_pos.z);

    // Don't want to project shadows on parallel surfaces
    if (std::abs(glm::dot(N, d)) < 0.001f) return false;

    float t = (offset - glm::dot(N, cat_pos)) / glm::dot(N, d);
    // if (t < 0) return false;                                     TODO: Check if it intersects? above? cat position

    glm::vec3 plane_pos = cat_pos + d*t;

    if (plane_pos.z < -0.0001f) return false;      // intersects below floor

    // Save intersection point
    intersect_pt = plane_pos;
    // intersect_pt.z += 1.0f;
    // std::cout << glm::to_string(intersect_pt) << std::endl;
    return true;
}

float PlayMode::get_surface_below_height(float &closest_dist) {
    float height = player.base.z;
    closest_dist = glm::length(player.base.z - glm::vec3(0,0,-0.0001f)); // account for minute differences

    switch_rooms(RoomType::LivingRoom);
    for (auto obj : *current_objects) {
        glm::vec3 intersect_pt;
        if (shadow_intersect(obj.transform, player.base, intersect_pt)) {
            // if (intersect_pt.z < height) {
            //     height = intersect_pt.z;
            // }
            float dist;
            if ((dist = glm::length(intersect_pt - player.base)) < closest_dist) {
                // std::cout << "*** " << obj.transform->name << ", dist = " << dist << ", closest = " << closest_dist << std::endl;
                closest_dist = dist;
                height = intersect_pt.z;
            }
        }
    }

    switch_rooms(RoomType::Kitchen);
    for (auto obj : *current_objects) {
        glm::vec3 intersect_pt;
        if (shadow_intersect(obj.transform, player.base, intersect_pt)) {
            float dist;
            if ((dist = glm::length(intersect_pt - player.base)) < closest_dist) {
                // std::cout << "*** " << obj.transform->name << ", dist = " << dist << ", closest = " << closest_dist << std::endl;
                closest_dist = dist;
                height = intersect_pt.z;
            }
        }
    }

    return height;
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
            // player.tip = player.transform->position;
            // player.tip.z += 1.0f;
            // player.base = player.transform->position;
            // player.base.z -= 1.0f;
            player.update_position(player.transform->position);
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
            if ((drawable.transform->name).find("Collide") != std::string::npos) {
                continue;       // Save these in a second pass
            }
            
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
            if (drawable.transform->name == "Vase") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&shattering);
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
    }

    else if (room_type == RoomType::Kitchen) {
        float floor_height = 0.0f;
        float island_x_min = 0, island_x_max = 0, island_y_min = 0, island_y_max = 0;

        for (auto &drawable : scene.drawables) {
            if (drawable.transform->name == "Player") continue;
            if ((drawable.transform->name).find("Collide") != std::string::npos) {
                continue;       // Save these in a second pass
            }
            
            CollisionType type = CollisionType::None;
            if (drawable.transform->name == "Stove Knob") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Stove Knob.001") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Stove Knob.002") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Stove Knob.003") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Faucet") type = CollisionType::Destroy;
            else if (drawable.transform->name == "Plate") type = CollisionType::Destroy;              // TODO: eventually push-offable
            else if (drawable.transform->name == "Plate.001") type = CollisionType::Destroy;          // TODO: eventually push-offable
            else if (drawable.transform->name == "Spoon") type = CollisionType::Steal;
            else if (drawable.transform->name == "Spoon.001") type = CollisionType::Steal;
            else if (drawable.transform->name == "Pan") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Pan.001") type = CollisionType::KnockOver;

            objects.push_back( RoomObject(drawable.transform, type) );

            if (drawable.transform->name == "Floor.001") {
                floor_height = objects.back().capsule.tip.z;
                kitchen_floor = drawable.transform;
            }
            if (drawable.transform->name == "Counter") {
                counter_transform = drawable.transform;
            }
            if (drawable.transform->name == "Island.001") {
                island_x_min = drawable.transform->bbox[1].x;
                island_x_max = drawable.transform->bbox[5].x;
                island_y_min = drawable.transform->bbox[1].y;
                island_y_max = drawable.transform->bbox[2].y;
            }
            if (drawable.transform->name == "Plate") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&shattering);
            }
            if (drawable.transform->name == "Plate.001") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&shattering);
            }
        }

        // ----- Search for FALLING objects to set start/end heights -----
        auto plate1_iter = find_if(objects.begin(), objects.end(),
                                [](const RoomObject &elem) { return elem.transform->name == "Plate"; });
        RoomObject &plate1 = *(plate1_iter);
        plate1.start_height = plate1.transform->position.z;
        plate1.end_height   = floor_height;
        plate1.x_min = island_x_min; plate1.x_max = island_x_max;
        plate1.y_min = island_y_min; plate1.y_max = island_y_max;

        auto plate2_iter = find_if(objects.begin(), objects.end(),
                                [](const RoomObject &elem) { return elem.transform->name == "Plate.001"; });
        RoomObject &plate2 = *(plate2_iter);
        plate2.start_height = plate2.transform->position.z;
        plate2.end_height   = floor_height;
        plate2.x_min = island_x_min; plate2.x_max = island_x_max;
        plate2.y_min = island_y_min; plate2.y_max = island_y_max;
    }

    // Applies for all rooms
    for (auto &obj: objects) {
        // Lookup after-collision drawable
        if ((obj.collision_type == CollisionType::PushOff) 
            || (obj.collision_type == CollisionType::KnockOver) 
            || (obj.collision_type == CollisionType::Destroy)) {
            
            auto obj_name = obj.name;
            auto collided_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                    [obj_name](const Scene::Drawable &elem) { return elem.transform->name == (obj_name + " Collided"); });
            if (collided_iter == scene.drawables.end()) std::cerr << "ERROR: Could not find post-collision resolution mesh associated with " << obj_name << std::endl;
            obj.reaction_drawables.push_back(*collided_iter);
            scene.drawables.erase(collided_iter);   // Safe to delete reaction drawables now
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

    // Get shadow transform 
    auto shadow_iter = find_if(living_room_scene.drawables.begin(), living_room_scene.drawables.end(),
                                        [](const Scene::Drawable &elem) { return elem.transform->name == "CatShadow"; });
    if (shadow_iter != living_room_scene.drawables.end()) {
        shadow.drawable = &(*shadow_iter);
        shadow.drawable->transform->position = living_room_floor->position;
    }
    // AddFrame(cat_scene, *(shadow.drawable));

    // ------------- Start background music! ---------------
    bg_loop = Sound::loop_3D(*bg_music, 0.1f, glm::vec3(0), 5.0f);
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
        // if (obj.name == "Rug") continue; // Rug is kinda blocky
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
        
        player.update_position(player.transform->position + movement);
        // shadow.update_position(player.base, &(living_room_floor->position.z));
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
    auto switchout_mesh = [&](RoomObject &resolved_obj) {        
        // std::cout << "===> Pos-collision, adding back " << resolved_obj.reaction_drawables[0].transform->name << std::endl;

        // First delete resolved object's mesh
        // TODO: use RemoveframeByName when current_scene is being used to differentiate collisions between rooms
        bool is_livingroom = false;
        if (!RemoveFrameByName(living_room_scene, resolved_obj.name)) {     // cannot find in living room
            if (!RemoveFrameByName(kitchen_scene, resolved_obj.name)) {     // cannot find in kitchen
                std::cerr << "ERROR: Cannot locate current object drawable: " << resolved_obj.name << std::endl;
            }
            else is_livingroom = false;
        }
        else is_livingroom = true;

        // Then add drawable of the resulting mesh
        if (is_livingroom)  living_room_scene.drawables.push_back(resolved_obj.reaction_drawables[0]);
        else                kitchen_scene.drawables.push_back(resolved_obj.reaction_drawables[0]);
    };

    auto pseudo_remove_bbox = [&](RoomObject &removed_obj) {
        // Save current bounding box
        for (auto i = 0; i < 8; i++) {
            removed_obj.orig_bbox[i] = removed_obj.transform->bbox[i];
            removed_obj.transform->bbox[i] = glm::vec3(-10000);
        }
        // Move capsule tip and base, to be reset later (TODO: write a class helper that does this)
        removed_obj.capsule.tip = glm::vec3(-10000);
        removed_obj.capsule.base = glm::vec3(-10000);
    };


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
        pseudo_remove_bbox(collision_obj);
        
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
        score += 5;
        collision_obj.done = true;
        player.swatting = false;

        switchout_mesh(collision_obj);
        pseudo_remove_bbox(collision_obj);

        if(collision_obj.has_sound) {
            Sound::play(*(*(collision_obj.samples[0])), 1.0f, 0.0f);
        }
    }
    // --------- Knock over object ---------
    else if (collision_obj.collision_type == CollisionType::KnockOver && !collision_obj.done) {
        score += 3;
        collision_obj.done = true;

        switchout_mesh(collision_obj);
        pseudo_remove_bbox(collision_obj);
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
        // player.transform->position = prev_player_position;
        // player.tip = prev_player_position;
        // player.tip.z += 1.0f;
        // player.base = prev_player_position;
        // player.base.z -= 1.0f;
        player.update_position(prev_player_position);
        // shadow.update_position(player.base, &(living_room_floor->position.z));
    }

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

                    switchout_mesh(obj);
                    if(collision_obj.has_sound) {
                        Sound::play(*(*(collision_obj.samples[0])), 1.0f, 0.0f);
                    }
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
        // shadow.update_position(player.transform->position, &(living_room_floor->position.z));
    } else { // just gravity
        player.transform->position.z = player.starting_height + 0.5f * gravity * player.air_time * player.air_time;
        // shadow.update_position(player.transform->position, &(living_room_floor->position.z));
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
        float top_height = living_room_floor->position.z;           // TODO generalize to previous surface height
        if (use_up_vec) {
            top_height = get_top_height(object_collide);
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

        // player.transform->position = new_pos;
        // player.tip = new_pos;
        // player.tip.z += 1.0f;
        // player.base = new_pos;
        // player.base.z -= 1.0f;
        player.update_position(new_pos);
        // shadow.update_position(player.base, &(top_height));

        player.jumping = false;
        player.air_time = 0.f;
        player.starting_height = player.transform->position.z;
    }

    // Update shadow position
    {
        float closest_dist = 0;
        float height = get_surface_below_height(closest_dist);
        shadow.update_position(player.base, height, closest_dist);
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

    glUseProgram(blob_shadow_texture_program->program);
	glUniform1i(blob_shadow_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(blob_shadow_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(blob_shadow_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
    glUniform1f(blob_shadow_texture_program->DEPTH_float, shadow.closest_dist);
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

    // Enable blending - suggestions here from http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-10-transparency/
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cat_scene.draw(*player.camera);
    living_room_scene.draw(*player.camera);
    kitchen_scene.draw(*player.camera);

    { // DISPLAY BOUNDING BOXES FOR DEBUG PURPOSES!!!!!
        glDisable(GL_DEPTH_TEST);
        DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));

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

        // for (auto &drawable : (*current_scene).drawables) {
        for (auto obj : living_room_objects) {
            // if (drawable.transform->name != "Table.005" && drawable.transform->name != "Key") continue;

            // draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            // draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

            // draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0xff, 0xff, 0xff));

            // top
            if (obj.transform->top_stand) {
                draw_lines.draw(obj.transform->bbox[5], obj.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[1], obj.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[2], obj.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[6], obj.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            }
            
            // bottom
            if (obj.transform->bot_stand) {
                draw_lines.draw(obj.transform->bbox[4], obj.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[0], obj.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[3], obj.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[7], obj.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            }
            
            // left
            if (obj.transform->left_stand) {
                draw_lines.draw(obj.transform->bbox[5], obj.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[6], obj.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[7], obj.transform->bbox[4], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
                draw_lines.draw(obj.transform->bbox[4], obj.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            }
            // right
            if (obj.transform->right_stand) {
                draw_lines.draw(obj.transform->bbox[1], obj.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[2], obj.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[3], obj.transform->bbox[0], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
                draw_lines.draw(obj.transform->bbox[0], obj.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            }

            // front
            if (obj.transform->front_stand) {
                draw_lines.draw(obj.transform->bbox[6], obj.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[2], obj.transform->bbox[3], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[3], obj.transform->bbox[7], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[7], obj.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
            }

            // back
            if (obj.transform->back_stand) {
                draw_lines.draw(obj.transform->bbox[5], obj.transform->bbox[1], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[1], obj.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[0], obj.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
                draw_lines.draw(obj.transform->bbox[4], obj.transform->bbox[5], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
            }

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
