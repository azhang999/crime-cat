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

GLuint walls_doors_floors_stairs_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > walls_doors_floors_stairs_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating walls_doors_floors_stairs Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("walls_doors_floors_stairs.pnct"), data_path("walls_doors_floors_stairs.boundbox"));
	walls_doors_floors_stairs_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint bedroom_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > bedroom_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating Bedroom Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("bedroom.pnct"), data_path("bedroom.boundbox"));
	bedroom_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint bathroom_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > bathroom_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating Bathroom Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("bathroom.pnct"), data_path("bathroom.boundbox"));
	bathroom_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint office_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > office_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    printf("Creating Office Meshes\n");
	MeshBuffer const *ret = new MeshBuffer(data_path("office.pnct"), data_path("office.boundbox"));
	office_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
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
        // if (transform->name == "CatShadow") {                                // TODO: This breaks! I don't know why! Better just to keep the shadow in the living room.
        //     drawable.pipeline = blob_shadow_texture_program_pipeline;
        //     drawable.pipeline.vao = living_room_meshes_for_blob_shadow_texture_program;
        // }
        // else {
        //     drawable.pipeline = lit_color_texture_program_pipeline;
        //     drawable.pipeline.vao = living_room_meshes_for_lit_color_texture_program;
        // }
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
            drawable.last_pass = true;
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

Load< Scene > walls_doors_floors_stairs_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("walls_doors_floors_stairs.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
		Mesh const &mesh = walls_doors_floors_stairs_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = walls_doors_floors_stairs_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Scene > bedroom_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("bedroom.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
		Mesh const &mesh = bedroom_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = bedroom_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Scene > bathroom_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("bathroom.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
		Mesh const &mesh = bathroom_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = bathroom_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Scene > office_scene_load(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("office.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        // printf("Mesh Name: %s\n", mesh_name.c_str());
		Mesh const &mesh = office_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = office_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

// source: https://freesound.org/people/m_delaparra/sounds/338018/
Load< Sound::Sample > shattering(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("shattering.wav"));
});
// source: https://freesound.org/people/InspectorJ/sounds/415765/
Load< Sound::Sample > tearing(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("tearing.wav"));
});
// source: https://freesound.org/people/XTYL33/sounds/68223/
Load< Sound::Sample > papers(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("papers.wav"));
});
// source: https://freesound.org/people/RoyalRose/sounds/560298/
Load< Sound::Sample > clink(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("clink.wav"));
});
// source: https://freesound.org/people/budek/sounds/513481/
Load< Sound::Sample > click(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("click.wav"));
});
// source: https://freesound.org/people/ChristiaanAckermann21100333/sounds/593726/
Load< Sound::Sample > pillow(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pillow.wav"));
});
// source: https://freesound.org/people/LG/sounds/73046/
Load< Sound::Sample > door(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("door.wav"));
});
// source: https://freesound.org/people/nicholasdaryl/sounds/563457/
Load< Sound::Sample > books(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("books.wav"));
});
// source: https://freesound.org/people/Debsound/sounds/168822/
Load< Sound::Sample > trophy(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("trophy.wav"));
});
// source: https://freesound.org/people/dmadridp/sounds/233476/
Load< Sound::Sample > typing(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("typing.wav"));
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

    for (auto room_type : current_rooms) {
        switch_rooms(room_type);
        for (auto obj : *current_objects) {
            glm::vec3 intersect_pt;
            if (shadow_intersect(obj.transform, player.base, intersect_pt)) {
                float dist;
                if ((dist = glm::length(intersect_pt - player.base)) < closest_dist) {
                    closest_dist = dist;
                    height = intersect_pt.z;
                }
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

        if (drawable.transform->top_stand) {
            float max_z = std::max({bbox.P2.z, bbox.P3.z, bbox.P6.z, bbox.P7.z});
            drawable.transform->bbox[1].z = max_z;
            drawable.transform->bbox[2].z = max_z;
            drawable.transform->bbox[5].z = max_z;
            drawable.transform->bbox[6].z = max_z;
        } else if (drawable.transform->bot_stand) {
            float max_z = std::max({bbox.P1.z, bbox.P4.z, bbox.P5.z, bbox.P8.z});
            drawable.transform->bbox[0].z = max_z;
            drawable.transform->bbox[3].z = max_z;
            drawable.transform->bbox[4].z = max_z;
            drawable.transform->bbox[7].z = max_z;
        } else if (drawable.transform->front_stand) {
            float max_z = std::max({bbox.P3.z, bbox.P4.z, bbox.P7.z, bbox.P8.z});
            drawable.transform->bbox[2].z = max_z;
            drawable.transform->bbox[3].z = max_z;
            drawable.transform->bbox[6].z = max_z;
            drawable.transform->bbox[7].z = max_z;
        } else if (drawable.transform->back_stand) {
            float max_z = std::max({bbox.P1.z, bbox.P2.z, bbox.P5.z, bbox.P6.z});
            drawable.transform->bbox[0].z = max_z;
            drawable.transform->bbox[1].z = max_z;
            drawable.transform->bbox[4].z = max_z;
            drawable.transform->bbox[5].z = max_z;
        } else if (drawable.transform->left_stand) {
            float max_z = std::max({bbox.P5.z, bbox.P6.z, bbox.P7.z, bbox.P8.z});
            drawable.transform->bbox[4].z = max_z;
            drawable.transform->bbox[5].z = max_z;
            drawable.transform->bbox[6].z = max_z;
            drawable.transform->bbox[7].z = max_z;
        } else if (drawable.transform->right_stand) {
            float max_z = std::max({bbox.P1.z, bbox.P2.z, bbox.P3.z, bbox.P4.z});
            drawable.transform->bbox[0].z = max_z;
            drawable.transform->bbox[1].z = max_z;
            drawable.transform->bbox[2].z = max_z;
            drawable.transform->bbox[3].z = max_z;
        }

        // auto transform = drawable.transform;
        // std::cout << transform->name << " "<< transform->top_stand << " " << transform->bot_stand 
        //                     << " " << transform->front_stand << " " << transform->back_stand
        //                     << " " << transform->left_stand << " " << transform->right_stand << std::endl;
        // std::cout << transform->name << " "<< transform->bbox[3].x << " " << transform->bbox[3].y 
        //                     << " " << transform->bbox[3].z << std::endl;
        if (drawable.transform->name == "Player") {
            player.transform_middle = drawable.transform;
            player.starting_height = player.transform_middle->position.z;
            player.update_position(player.transform_middle->position);
            player.starting_height = player.transform_middle->position.z;
        } else if (drawable.transform->name == "PlayerFront") {
            player.transform_front = drawable.transform;
        } else if (drawable.transform->name == "Paw") {
            player.paw = drawable.transform;
        } else if (drawable.transform->name == "Mouth") {
            player.mouth = drawable.transform;
        } else if (drawable.transform->name.find("Mouth") != std::string::npos) { // has word mouth in it
            // scale them to 0 (hide them)
            // drawable.transform->scale = glm::vec3(0.f);
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

void PlayMode::generate_wdfs_objects(Scene &scene, std::vector<RoomObject> &objects) {
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name.find("Door") == std::string::npos && 
            drawable.transform->name.find("Pass") == std::string::npos) {
            CollisionType type = CollisionType::None;
            objects.push_back( RoomObject(drawable.transform, type) );
        }

        if (drawable.transform->name == "First Floor") {
            living_room_floor = drawable.transform;
        }
    }
}

void PlayMode::generate_living_room_objects(Scene &scene, std::vector<RoomObject> &objects) {
    float rug_height = 0.0f;
    float sidetable_x_min = 0, sidetable_x_max = 0, sidetable_y_min = 0, sidetable_y_max = 0;

    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Player") continue;
        if ((drawable.transform->name).find("Collide") != std::string::npos) {
            continue;       // Save these in a second pass
        }
        
        CollisionType type = CollisionType::None;  
        if (drawable.transform->name == "Vase")             type = CollisionType::PushOff;
        else if (drawable.transform->name == "Key")         type = CollisionType::Steal;
        else if (drawable.transform->name == "Mug")         type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Pillow")      type = CollisionType::Destroy;
        else if (drawable.transform->name == "Pillow.001")  type = CollisionType::Destroy;
        else if (drawable.transform->name == "Magazine")    type = CollisionType::KnockOver;

        objects.push_back( RoomObject(drawable.transform, type) );

        if (drawable.transform->name == "Rug") rug_height = objects.back().capsule.tip.z;
        if (drawable.transform->name == "SideTable") {
            sidetable_x_min = drawable.transform->bbox[1].x;
            sidetable_x_max = drawable.transform->bbox[5].x;
            sidetable_y_min = drawable.transform->bbox[1].y;
            sidetable_y_max = drawable.transform->bbox[2].y;
        }
        if (drawable.transform->name == "Vase") {
            objects.back().given_speed = 3.0f;
            objects.back().has_sound = true;
            objects.back().samples.push_back(&shattering);
            objects.back().spin = true;
        }

        if (drawable.transform->name == "Key") {
            objects.back().capsule.radius = 0.1f;
            objects.back().capsule.height = 0.6f;
        }
        
        if (drawable.transform->name == "Pillow") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&tearing);
        }
        if (drawable.transform->name == "Pillow.001") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&tearing);
        }
        if (drawable.transform->name == "Mug") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&clink);
        }
        if (drawable.transform->name == "Magazine") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&papers);
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

void PlayMode::generate_kitchen_objects(Scene &scene, std::vector<RoomObject> &objects) {
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
            else if (drawable.transform->name == "Plate") type = CollisionType::PushOff;              // TODO: eventually push-offable
            else if (drawable.transform->name == "Plate.001") type = CollisionType::PushOff;          // TODO: eventually push-offable
            else if (drawable.transform->name == "Spoon") type = CollisionType::Steal;
            else if (drawable.transform->name == "Spoon.001") type = CollisionType::Steal;
            else if (drawable.transform->name == "Pan") type = CollisionType::KnockOver;
            else if (drawable.transform->name == "Pan.001") type = CollisionType::KnockOver;

            objects.push_back( RoomObject(drawable.transform, type) );

            if (drawable.transform->name == "Spoon.001") {
                objects.back().capsule.height = 0.6f;
            }
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
                objects.back().given_speed = 6.0f;
            }
            if (drawable.transform->name == "Plate.001") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&shattering);
                objects.back().given_speed = 6.0f;
            }
            if (drawable.transform->name == "Pan") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&clink);
            }
            if (drawable.transform->name == "Pan.001") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&clink);
            }
            if (drawable.transform->name == "Faucet") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&click);
            }
            if (drawable.transform->name == "Stove Knob") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&click);
            }
            if (drawable.transform->name == "Stove Knob.001") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&click);
            }
            if (drawable.transform->name == "Stove Knob.002") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&click);
            }
            if (drawable.transform->name == "Stove Knob.003") {
                objects.back().has_sound = true;
                objects.back().samples.push_back(&click);
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

void PlayMode::generate_bedroom_objects(Scene &scene, std::vector<RoomObject> &objects) {
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Player") continue;
        if ((drawable.transform->name).find("Collide") != std::string::npos) {
            continue;       // Save these in a second pass
        }
        
        CollisionType type = CollisionType::None;  
        if (drawable.transform->name == "Closet")             type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Slipper")         type = CollisionType::Steal;
        else if (drawable.transform->name == "Slipper.001")         type = CollisionType::Steal;
        else if (drawable.transform->name == "Switch")      type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Sock")  type = CollisionType::Steal;
        else if (drawable.transform->name == "Meds")    type = CollisionType::Steal;
        else if (drawable.transform->name == "Alarm Clock")    type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Bed Pillow")    type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Bed Pillow.001")    type = CollisionType::KnockOver;

        objects.push_back( RoomObject(drawable.transform, type) );

        if (drawable.transform->name == "Sock") {
            objects.back().capsule.radius = 0.1f;
            objects.back().capsule.height = 0.5f;
        }
        if (drawable.transform->name == "Slipper") {
            objects.back().capsule.radius = 0.1f;
            objects.back().capsule.height = 0.5f;
        }
        if (drawable.transform->name == "Slipper.001") {
            objects.back().capsule.radius = 0.1f;
            objects.back().capsule.height = 0.5f;
        }
        if (drawable.transform->name == "Closet") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&door);
        }
        if (drawable.transform->name == "Switch") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&click);
        }
        if (drawable.transform->name == "Alarm Clock") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&click);
        }
        if (drawable.transform->name == "Bed Pillow") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&pillow);
        }
        if (drawable.transform->name == "Bed Pillow.001") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&pillow);
        }
    }
}

void PlayMode::generate_bathroom_objects(Scene &scene, std::vector<RoomObject> &objects) {
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Player") continue;
        if ((drawable.transform->name).find("Collide") != std::string::npos) {
            continue;       // Save these in a second pass
        }
        
        CollisionType type = CollisionType::None;  
        if (drawable.transform->name == "Toothbrush")                       type = CollisionType::Steal;
        else if (drawable.transform->name == "Bathroom Sink Faucet")        type = CollisionType::Destroy;
        else if (drawable.transform->name == "Bathtub Faucet")              type = CollisionType::Destroy;
        else if (drawable.transform->name == "Towel")                       type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Toilet Paper Roll")           type = CollisionType::Steal;

        objects.push_back( RoomObject(drawable.transform, type) );
        
        if (drawable.transform->name == "Toothbrush") {
            objects.back().capsule.radius = 0.1f;
            objects.back().capsule.height = 0.5f;
        }
        if (drawable.transform->name == "Bathroom Sink Faucet") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&click);
        }
        if (drawable.transform->name == "Bathtub Faucet") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&click);
        }
        if (drawable.transform->name == "Towel") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&pillow);
        }
    }
}

void PlayMode::generate_office_objects(Scene &scene, std::vector<RoomObject> &objects) {
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Player") continue;
        if ((drawable.transform->name).find("Collide") != std::string::npos) {
            continue;       // Save these in a second pass
        }
        
        CollisionType type = CollisionType::None;  
        if (drawable.transform->name == "Desk Lamp")                       type = CollisionType::Destroy;
        else if (drawable.transform->name == "Notebook")        type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Laptop Screen")              type = CollisionType::KnockOver;
        else if (drawable.transform->name == "Trophy")                       type = CollisionType::PushOff;
        else if (drawable.transform->name == "Books")           type = CollisionType::Destroy;
        else if (drawable.transform->name == "Armchair Seat")           type = CollisionType::Destroy;

        objects.push_back( RoomObject(drawable.transform, type) );

        if (drawable.transform->name == "Desk Lamp") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&trophy);
        }
        if (drawable.transform->name == "Notebook") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&papers);
        }
        if (drawable.transform->name == "Laptop") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&typing); // can't hear this one?
        }
        if (drawable.transform->name == "Trophy") {
            objects.back().given_speed = 3.0f;
            // doesn't play sound when trophy hits the floor
            objects.back().has_sound = true;
            objects.back().samples.push_back(&trophy);
            objects.back().spin = true;
        }
        if (drawable.transform->name == "Books") {
            objects.back().has_sound = true;
            objects.back().samples.push_back(&books);
        }
    }
}

void PlayMode::generate_room_objects(Scene &scene, std::vector<RoomObject> &objects, RoomType room_type) {
    switch (room_type) {
        case RoomType::LivingRoom: {
            generate_living_room_objects(scene, objects);
            break;
        }
        case RoomType::Kitchen: {
            generate_kitchen_objects(scene, objects);
            break;
        }
        case RoomType::WallsDoorsFloorsStairs: {
            generate_wdfs_objects(scene, objects);
            break;
        }
        case RoomType::Bedroom: {
            generate_bedroom_objects(scene, objects);
            break;
        }
        case RoomType::Bathroom: {
            generate_bathroom_objects(scene, objects);
            break;
        }
        case RoomType::Office: {
            generate_office_objects(scene, objects);
            break;
        }
        default: {
            printf("ERROR Room Type: %d not implemented yet\n", room_type);
            exit(1);
        }
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
    switch (room_type) {
        case RoomType::LivingRoom: {
            current_scene   = &living_room_scene;
            current_objects = &living_room_objects;
            break;
        }
        case RoomType::Kitchen: {
            current_scene   = &kitchen_scene;
            current_objects = &kitchen_objects;
            break;
        }
        case RoomType::WallsDoorsFloorsStairs: {
            current_scene   = &wdfs_scene;
            current_objects = &wdfs_objects;
            break;
        }
        case RoomType::Bedroom: {
            current_scene   = &bedroom_scene;
            current_objects = &bedroom_objects;
            break;
        }
        case RoomType::Bathroom: {
            current_scene   = &bathroom_scene;
            current_objects = &bathroom_objects;
            break;
        }
        case RoomType::Office: {
            current_scene   = &office_scene;
            current_objects = &office_objects;
            break;
        }
        default: {
            printf("ERROR (switch_room) Room Type: %d not implemented yet\n", room_type);
            exit(1);
            break;
        }
    }
}

PlayMode::PlayMode() : 
    cat_scene(*cat_scene_load), 
    living_room_scene(*living_room_scene_load), 
    kitchen_scene(*kitchen_scene_load),
    wdfs_scene(*walls_doors_floors_stairs_scene_load),
    bedroom_scene(*bedroom_scene_load),
    bathroom_scene(*bathroom_scene_load),
    office_scene(*office_scene_load) {
    
    GenerateBBox(cat_scene, cat_meshes);

    // remove player capsule from being drawn
    RemoveFrameByName(cat_scene, "Player");
    RemoveFrameByName(cat_scene, "PlayerFront");
    RemoveFrameByName(cat_scene, "Paw");
    RemoveFrameByName(cat_scene, "Mouth");

    player_walking.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    player_up_jump.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 1000000.f}; // don't want to cycle
    player_down_jump.frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 1000000.f}; // don't want to cycle
    player_swat.frame_times = {0.1f, 0.1f, 100000.f}; // don't want to cycle

    GetFrames(cat_scene, player_walking, "Walk");
    GetFrames(cat_scene, player_up_jump, "UpJump");
    GetFrames(cat_scene, player_down_jump, "DownJump");
    GetFrames(cat_scene, player_swat, "Swat");

    // remove held items from scene
    bool done = false;
    while (!done) {
        auto frame_iter = find_if(cat_scene.drawables.begin(), cat_scene.drawables.end(),
                                [](const Scene::Drawable & elem) { return elem.transform->name.find("Mouth") != std::string::npos; });
        if (frame_iter == cat_scene.drawables.end()) {
            done = true;
        } else {
            player_held_items.push_back(*frame_iter);
            cat_scene.drawables.erase(frame_iter);
        }
    }

    // start cat with Walk0 fame
    AddFrame(cat_scene, player_walking.frames[0]);

    if (cat_scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(cat_scene.cameras.size()));
	player.camera = &cat_scene.cameras.front();

    GenerateBBox(living_room_scene, living_room_meshes);
    GenerateBBox(kitchen_scene, kitchen_meshes);
    GenerateBBox(wdfs_scene, walls_doors_floors_stairs_meshes);
    GenerateBBox(bedroom_scene, bedroom_meshes);
    GenerateBBox(bathroom_scene, bathroom_meshes);
    GenerateBBox(office_scene, office_meshes);

    generate_room_objects(living_room_scene, living_room_objects, RoomType::LivingRoom);
    generate_room_objects(kitchen_scene, kitchen_objects, RoomType::Kitchen);
    generate_room_objects(wdfs_scene, wdfs_objects, RoomType::WallsDoorsFloorsStairs);
    generate_room_objects(bedroom_scene, bedroom_objects, RoomType::Bedroom);
    generate_room_objects(bathroom_scene, bathroom_objects, RoomType::Bathroom);
    generate_room_objects(office_scene, office_objects, RoomType::Office);

    // ----- Start in living room -----
    switch_rooms(RoomType::LivingRoom);

    // Get shadow transform 
    auto shadow_iter = find_if(living_room_scene.drawables.begin(), living_room_scene.drawables.end(),
                                        [](const Scene::Drawable &elem) { return elem.transform->name == "CatShadow"; });
    if (shadow_iter != living_room_scene.drawables.end()) {
        shadow.drawable = &(*shadow_iter);
        shadow.drawable->transform->position = living_room_floor->position;
    } else {
        std::cout << "************** OI MATE WHERE'S THE SHADOW **************" << std::endl;
    }

    // ------------- Setup text rendering ---------------
    game_text.PLAYMODE = true;
    game_text.init_state(script_path);
    game_text.fill_state();
    // ------------- Start background music! ---------------
    // bg_loop = Sound::loop_3D(*bg_music, 0.1f, glm::vec3(0), 5.0f);
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
        } else if (evt.key.keysym.sym == SDLK_i) {  // Switches back to InstrucMode until the user escapes
            Mode::set_current(instruct_mode);
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
		} else {
            swat.pressed = true;
        }
		return true;
    } 
    // else if (evt.type == SDL_MOUSEBUTTONUP) {
    //     swat.pressed = false;
    //     return true;
	else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);

            // face left, face right
            phi -= motion.x;
            theta -= motion.y;
			theta = -std::min(-theta, 0.55f * 3.1415926f);
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
    for (auto room_type : current_rooms) {
        switch_rooms(room_type);
        for (auto obj : *current_objects) {
            // if (obj.name == "Rug") continue; // Rug is kinda blocky      
            if (obj.name == current_obj.name) continue;

            auto capsule = current_obj.capsule;
            SurfaceType surface;

            if (capsule_bbox_collision(capsule.tip, capsule.base, capsule.radius, obj.transform->bbox, &surface, pen_normal, pen_depth)) {
                return obj.name;
            }
        }
    }
    return "";
}

std::string PlayMode::paw_collide() {
   for (auto room_type : current_rooms) {
        switch_rooms(room_type);

        for (auto obj : *current_objects) {
            if (obj.collision_type != CollisionType::Steal && obj.collision_type != CollisionType::Destroy) continue;

            glm::vec3 paw_tip = player.paw->make_local_to_world() * glm::vec4(player.paw->position, 1.0f);
            glm::vec3 paw_base = paw_tip;
            paw_base.z += 1.0f;
            paw_tip.z -= 1.0f;
            // temps are throw aways
            SurfaceType temp_surface;
            glm::vec3 temp_pen_normal;
            float temp_pen_depth;
            if (capsule_bbox_collision(paw_tip, paw_base, player.radius, obj.transform->bbox, &temp_surface, &temp_pen_normal, &temp_pen_depth)) {
                return obj.name;
            }
        }

    }
    return "";
}

Scene::Transform *PlayMode::collide() {
    num_collide_objs = 0;
    SurfaceType surface;

    Scene::Transform *collide_obj = nullptr;

    // save the vector that is most like (0, 0, 1)
    // save that collision into penetration_normal, penetration_depth
    glm::vec3 best_pen_normal = glm::vec3(0.f);
    float best_pen_depth = 0.f;

    for (auto room_type : current_rooms) {
        switch_rooms(room_type);

        for (auto obj : *current_objects) {
            // skip over thin/small things
            // living room
            if (obj.name == "Rug") continue;

            // kitchen
            if (obj.name == "Mat") continue;
            if (obj.name == "Spider Burner") continue;
            if (obj.name == "Spider Burner.001") continue;
            if (obj.name == "Spider Burner.002") continue;
            if (obj.name == "Spider Burner.003") continue;

            // bathroom
            if (obj.name == "Bath Mat") continue;
            if (obj.name == "Bath Mat.001") continue;

            // office
            if (obj.name == "Desk Mat") continue;
            if (obj.name == "Pencil") continue;

            if (obj.collision_type == CollisionType::Steal) continue;
            // printf("transform_front:.....\n");
            // printf("transform_front: %f %f %f\n", player.transform_front->position.x, player.transform_front->position.y, player.transform_front->position.z);
            glm::vec3 player_tip = player.transform_front->make_local_to_world() * glm::vec4(player.transform_front->position, 1.0f);
            glm::vec3 player_base = player_tip;
            player_tip.z += 1.0f;
            player_base.z -= 1.0f;
            if (capsule_bbox_collision(player_tip, player_base, player.radius, obj.transform->bbox, &surface, &penetration_normal, &penetration_depth)) {
                // printf("collided with: %s\n", obj.name.c_str());
                num_collide_objs++;
                collide_obj = obj.transform;
                if (num_collide_objs == 1 || is_almost_up_vec(penetration_normal)) {
                    best_pen_normal = penetration_normal;
                    best_pen_depth = penetration_depth;
                }
            }

            player_tip = player.transform_middle->position;
            player_tip.z += 1.0f;
            player_base = player.transform_middle->position;
            player_base.z -= 1.0f;
            if (capsule_bbox_collision(player_tip, player_base, player.radius, obj.transform->bbox, &surface, &penetration_normal, &penetration_depth)) {
                num_collide_objs++;
                collide_obj = obj.transform;
                if (num_collide_objs == 1 || is_almost_up_vec(penetration_normal)) {
                    best_pen_normal = penetration_normal;
                    best_pen_depth = penetration_depth;
                }
            }
        }

    }

    penetration_normal = best_pen_normal;
    penetration_depth = best_pen_depth;

    return collide_obj;
}

// ROOM OBJECTS COLLISION AND MOVEMENT START ------------------------
void PlayMode::interact_with_objects(float elapsed, std::string object_collide_name, glm::vec3 player_motion) {

    auto switchout_mesh = [&](RoomObject &resolved_obj) {        
        // std::cout << "===> Pos-collision, adding back " << resolved_obj.reaction_drawables[0].transform->name << std::endl;

        // First delete resolved object's mesh
        // TODO: use RemoveframeByName when current_scene is being used to differentiate collisions between rooms
        bool deleted = false;
        for (auto room_type : current_rooms) {
            switch_rooms(room_type);
            if (RemoveFrameByName(*current_scene, resolved_obj.name)) {
                current_scene->drawables.push_back(resolved_obj.reaction_drawables[0]);
                deleted = true;
                break;
            }
        }

        if (!deleted) {
            std::cerr << "ERROR: Cannot locate current object drawable: " << resolved_obj.name << std::endl;
        }
    };

    auto pseudo_remove_bbox = [&](RoomObject &removed_obj) {
        // Save current bounding box
        for (auto i = 0; i < 8; i++) {
            // removed_obj.orig_bbox[i] = removed_obj.transform->bbox[i];
            removed_obj.transform->bbox[i] = glm::vec3(-10000);
        }
        // Move capsule tip and base, to be reset later (TODO: write a class helper that does this)
        removed_obj.capsule.tip = glm::vec3(-10000);
        removed_obj.capsule.base = glm::vec3(-10000);
    };

    auto restore_removed_bbox = [&](RoomObject &removed_obj) {
        // Restore current bounding box
        for (auto i = 0; i < 8; i++) {
            removed_obj.transform->bbox[i] = removed_obj.orig_bbox[i] + (removed_obj.transform->position - removed_obj.orig_pos);
        }
    };

    // check for paw
    if (swat.pressed) {
        if (player.swatting && !player.holding) {
            std::string swat_obj_name = paw_collide();
            if (swat_obj_name != "") {
                object_collide_name = swat_obj_name;
            }
        } else if (player.swatting && player.holding) {
            // put item down
            // printf("putting down %s\n", player.held_obj->transform->name.c_str());
            object_collide_name = "";
            glm::vec3 abs_transform_front_pos = player.transform_front->make_local_to_world() * glm::vec4(player.transform_front->position, 1.0f);
            glm::vec3 t_offset = abs_transform_front_pos - player.transform_middle->position;
            t_offset.z = 0.f;
            t_offset = glm::normalize(t_offset);
            player.held_obj->transform->position = abs_transform_front_pos + (t_offset * 0.2f);
            player.held_obj->transform->position += glm::vec3(0.f, 0.f, 0.6f);
            restore_removed_bbox(*player.held_obj);
            player.held_obj = nullptr;
            player.holding = false;
        }
    }


    // player has picked up object
    bool found_object = false;
    std::vector<RoomObject>::iterator collision_obj_iter;
    for (auto room_type : current_rooms) {
        switch_rooms(room_type);
        collision_obj_iter = find_if((*current_objects).begin(), (*current_objects).end(),
                                        [object_collide_name](const RoomObject &elem) { return elem.name == object_collide_name; });
        if (collision_obj_iter != (*current_objects).end()) {
            found_object = true;
            break;
        }
    }

    if (found_object) {
        RoomObject &collision_obj = *(collision_obj_iter);

        // parse out every including and past . in the name
        size_t period_pos = 0;
        //SOURCE: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
        std::string parsed_name = "";

        parsed_name = collision_obj.transform->name;
        if ((period_pos = collision_obj.transform->name.find(".")) != std::string::npos) {
            parsed_name = collision_obj.transform->name.substr(0, period_pos);;
        }

        switch (collision_obj.collision_type) {
            case CollisionType::Steal: {
                if (!player.swatting) break; // need to be swatting
                if (!collision_obj.done) {
                    score += 3;
                    display_collide = true;
                    collide_msg_time = 3.0f;
                    collide_label = "+3 " + parsed_name;
                    collision_obj.done = true;
                }
                player.swatting = false;
                player.swatting_timer = 0.f;

                player.held_obj = &collision_obj;
                player.holding = true;

                // Save current scale
                // collision_obj.orig_scale = collision_obj.transform->scale;
                // collision_obj.transform->scale = glm::vec3(0);
                // Save current position
                collision_obj.prev_position = collision_obj.transform->position;
                collision_obj.transform->position = glm::vec3(-10000);
                pseudo_remove_bbox(collision_obj);
                break;
            }
            case CollisionType::Destroy: {
                if (collision_obj.done) break;
                if (!player.swatting) break; // need to be swatting
                score += 5;
                display_collide = true;
                collide_msg_time = 3.0f;
                collide_label = "+5 " + parsed_name;
                collision_obj.done = true;
                player.swatting = false;

                switchout_mesh(collision_obj);
                pseudo_remove_bbox(collision_obj);

                if(collision_obj.has_sound) {
                    Sound::play(*(*(collision_obj.samples[0])), 1.0f, 0.0f);
                }
                break;
            }
            case CollisionType::KnockOver: {
                if (collision_obj.done) break;
                score += 3;
                display_collide = true;
                collide_msg_time = 3.0f;
                collide_label = "+3 " + parsed_name;
                collision_obj.done = true;

                switchout_mesh(collision_obj);
                pseudo_remove_bbox(collision_obj);

                if(collision_obj.has_sound) {
                    Sound::play(*(*(collision_obj.samples[0])), 1.0f, 0.0f);
                }
                break;
            }
            case CollisionType::PushOff: {
                if (collision_obj.done) break;
                if (glm::length(player_motion) > 0.f) {
                    collision_obj.move_dir = glm::normalize(player_motion);
                    collision_obj.move_dir.z = 0.f;
                    collision_obj.is_moving = true;
                    collision_obj.speed = collision_obj.given_speed;
                    // printf("dir x:%f y:%f z:%f\n", collision_obj.move_dir.x, collision_obj.move_dir.y, collision_obj.move_dir.z);
                }
                break;
            }
            default: {
                // printf("Unknown CollisionType detected\n");
            }
        }
    }
    

    // ##################### Resolve remaining collision behavior #####################
    // simulate object motion
    for (auto room_type : current_rooms) {
        switch_rooms(room_type);

        for (auto &obj : *current_objects) {
            if (isnan(obj.transform->position.x) || isnan(obj.transform->position.y) || isnan(obj.transform->position.z)) {
                printf("ERROR: OBJECT IS NAN: %s %f %f %f\n", obj.transform->name.c_str(), obj.transform->position.x, obj.transform->position.y, obj.transform->position.z);
                // exit(1);
            }

            if (obj.collision_type == CollisionType::PushOff && !obj.done) {
                // execute horizontal movement
                obj.transform->position += obj.move_dir * elapsed * obj.speed;
                obj.capsule.tip = obj.transform->position;
                obj.capsule.tip.z += obj.capsule.height/2;
                obj.capsule.base = obj.transform->position;
                obj.capsule.base.z  -= obj.capsule.height/2;

                obj.speed -= elapsed * 5.0f;
                if (obj.speed < 0.01f) {
                    obj.speed = 0.f;
                }

                // check if horizontal movement caused collision
                std::string horizontal_collision_name = capsule_collide(obj, &obj.pen_dir, &obj.pen_depth);
                if (horizontal_collision_name != "") {
                    // reflect
                    obj.move_dir = glm::normalize(obj.pen_dir);
                    
                    glm::vec3 offset = ((obj.pen_depth + 0.1f) * obj.pen_dir);
                    offset.z = 0.f;
                    obj.transform->position += offset;
                }

                 // gravity - break if hits floor
                obj.transform->position.z -= elapsed * 6.0f;
                obj.capsule.tip = obj.transform->position;
                obj.capsule.tip.z += obj.capsule.height/2;
                obj.capsule.base = obj.transform->position;
                obj.capsule.base.z  -= obj.capsule.height/2;

                bool call_restore = true;
                std::string vertical_collision_name = capsule_collide(obj, &obj.pen_dir, &obj.pen_depth);
                if (vertical_collision_name != "") {
                    if (std::abs(obj.orig_pos.z - obj.transform->position.z) > 1.0f) {
                        // fell alot
                        score += 7;
                        // parse out every including and past . in the name
                        size_t period_pos = 0;
                        //SOURCE: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
                        std::string parsed_name = obj.transform->name;
                        if ((period_pos = obj.transform->name.find(".")) != std::string::npos) {
                            parsed_name = obj.transform->name.substr(0, period_pos);;
                        }
                        collide_label = "+7 " + parsed_name;
                        collide_msg_time = 3.0f;
                        obj.collided = true;  // prevents user from gaining more points
                        obj.done = true;
                        obj.transform->rotation = obj.orig_rotation;

                        switchout_mesh(obj);
                        pseudo_remove_bbox(obj);
                        call_restore = false;
                        if(obj.has_sound) {
                            Sound::play(*(*(obj.samples[0])), 1.0f, 0.0f);
                        }
                    } else {
                        // hasn't fallen that much - undo grav
                        obj.transform->position += ((obj.pen_depth + 0.00001f) * obj.pen_dir);
                    }
                } else {
                    // give object some rotation
                    if (obj.spin && std::abs(obj.orig_pos.z - obj.transform->position.z) > 0.1f) {
                        obj.transform->rotation *= glm::angleAxis(9.0f * elapsed, glm::vec3(0, 1, 0));
                        obj.transform->rotation *= glm::angleAxis(9.0f * elapsed, glm::vec3(1, 0, 0));
                        obj.transform->rotation *= glm::angleAxis(9.0f * elapsed, glm::vec3(0, 0, 1));
                    }
                }

                if (call_restore) {
                    // update bbox with new_pos - orig_pos
                    restore_removed_bbox(obj);
                }


            } else if (obj.collision_type == CollisionType::Steal) {
                if (!player.holding || (player.held_obj->transform->name != obj.transform->name)) {
                    // gravity
                    glm::vec3 orig_pos = obj.transform->position;

                    obj.transform->position.z -= elapsed * 6.0f;
                    obj.capsule.tip = obj.transform->position;
                    obj.capsule.tip.z += obj.capsule.height/2;
                    obj.capsule.base = obj.transform->position;
                    obj.capsule.base.z  -= obj.capsule.height/2;

                    std::string vertical_collision_name = capsule_collide(obj, &obj.pen_dir, &obj.pen_depth);
                    if (vertical_collision_name != "") {
                        if (vertical_collision_name == "Cat Bed") {
                            // TODO: add meow sound
                            score += 10;
                            collide_label = "+10 New Toy";
                            collide_msg_time = 3.0f;
                            display_collide = true;
                            obj.transform->position = glm::vec3(1000.f);
                        } else if (vertical_collision_name == "Toilet.002") {
                            // TODO: add splash sound
                            score += 10;
                            collide_label = "+12 Splash";
                            collide_msg_time = 3.0f;
                            display_collide = true;
                            obj.transform->position = glm::vec3(1000.f);
                        } else {
                            obj.transform->position = orig_pos;
                        }
                    }

                    restore_removed_bbox(obj);
                }
            }
        }
    }

}

// ROOM OBJECTS COLLISION AND MOVEMENT END --------------------------

void PlayMode::update(float elapsed) {
    if (game_over) return;

    if (elapsed == 0.f || elapsed > 0.5f) {
        printf("ERROR elapsed time is %f\n", elapsed);
        // exit(1);
    }

    game_timer.seconds -= elapsed;
    if (game_timer.seconds <= 0.f) {
        game_over = true;
        game_text.GAMEOVER = true;
        game_timer.seconds = 0.f;
        return;
    }

    if (display_collide) {
        collide_msg_time -= elapsed;
        if (collide_msg_time <= 0.f) {
            display_collide = false;
            collide_msg_time = 0.f;
        }
    }
    
    glm::vec3 prev_player_position = player.transform_middle->position;
    glm::quat prev_player_rotation = player.transform_middle->rotation;

	//move player:
    //combine inputs into a move:
    constexpr float ground_speed = 8.0f;
    constexpr float air_speed = 5.0f;
    glm::vec2 move = glm::vec2(0.0f);

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
    glm::vec3 movement;
    movement.z = 0.f;
    if (move != glm::vec2(0.0f)) {
        if (player.jumping) {
            move = glm::normalize(move) * air_speed * elapsed;
        } else {
            move = glm::normalize(move) * ground_speed * elapsed;
        }
        movement = player.transform_middle->make_local_to_world() * glm::vec4(move.x, move.y, 0.f, 1.f) - player.transform_middle->position;
        player.transform_middle->position += movement;
        player.update_position(player.transform_middle->position);
        // shadow.update_position(player.base, &(living_room_floor->position.z));
    }

    { // rotate player
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

        if (left.pressed && !right.pressed)
            player.transform_middle->rotation *= glm::angleAxis(3.0f * elapsed, up);
        if (!left.pressed && right.pressed)
            player.transform_middle->rotation *= glm::angleAxis(-3.0f * elapsed, up);
    }

    is_side_ = true;
    auto object_collide = collide();
    is_side_ = false;
    std::string object_collide_name = "";
    if (object_collide != nullptr) {
        object_collide_name = object_collide->name;
    }

    glm::vec3 player_motion = movement;
    // if (object_collide_name != "") {
    //     printf("%s\n", object_collide_name.c_str());
    // }
    interact_with_objects(elapsed, object_collide_name, player_motion);

    // --------- * collision with non interactable object occurred * ---------
    if (object_collide_name != "") { // undo movement
        // SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
        // Modify player velocity to slide on contact surface:
        glm::vec3 offset = ((penetration_depth + 0.0001f) * penetration_normal);
        offset.z = 0.f;

        player.transform_middle->position += offset;
        player.update_position(player.transform_middle->position);

        int prev_num_collide_objs = num_collide_objs;
        auto new_collide_side = collide();
        if (new_collide_side != nullptr || prev_num_collide_objs > 1) {
            player.transform_middle->position = prev_player_position;
            player.transform_middle->rotation = prev_player_rotation;
            player.update_position(prev_player_position);
        }
    }


    // ######################### Resolve falling player #########################

    prev_player_position = player.transform_middle->position;

    player.air_time += elapsed;
    if (player.jumping) { // jumping
        player.transform_middle->position.z = player.starting_height + player.init_up_v * player.air_time + 0.5f * gravity * player.air_time * player.air_time;
    } else { // just gravity
        player.transform_middle->position.z = player.starting_height + 0.5f * gravity * player.air_time * player.air_time;
    }

    player.tip = player.transform_middle->position;
    player.tip.z += 1.0f;
    player.base = player.transform_middle->position;
    player.base.z -= 1.0f;

    object_collide = collide();
    object_collide_name = "";
    if (object_collide != nullptr) {
        object_collide_name = object_collide->name;
    }

    if (object_collide_name != "") {
        bool use_up_vec = is_almost_up_vec(penetration_normal);
        // printf("use_up_vec:%d\n", use_up_vec);
        glm::vec3 new_pos;
        //float top_height = living_room_floor->position.z;           // TODO generalize to previous surface height
        if (use_up_vec) {
            //top_height = get_top_height(object_collide);

            glm::vec3 offset = ((penetration_depth + 0.0001f) * glm::normalize(penetration_normal));
            // offset.z = 0;// ((penetration_depth + 0.000001f) * glm::normalize(penetration_normal).z);
            new_pos = player.transform_middle->position + offset;


            // new_pos =  player.transform_middle->position;
            // new_pos.z = top_height + 1.00001f;
        } else {
            glm::vec3 offset = ((penetration_depth + 0.25f) * glm::normalize(penetration_normal));
            offset.z = 0;// ((penetration_depth + 0.000001f) * glm::normalize(penetration_normal).z);
            new_pos = player.transform_middle->position + offset;
        }

        // place cat slightly above surface
        // float penetration_depth = player.radius - glm::length(intersection_vec);
        // float top_height = get_top_height(object_collide);
        // auto new_pos =  player.transform->position + ((penetration_depth + 0.000001f) * glm::normalize(intersection_vec));
        // auto new_pos = prev_player_position;
        // new_pos.z = top_height + 1.00001f;
        player.jumping = false;
        player.air_time = 0.f;
        player.transform_middle->position = new_pos;
        player.update_position(player.transform_middle->position);
        player.starting_height = player.transform_middle->position.z;
    }

    // Update shadow position
    {
        float closest_dist = 0;
        float height = get_surface_below_height(closest_dist);
        shadow.update_position(player.base, height, closest_dist);
    }

    // remove all mouth related things from cat_scene
    bool done = false;
    while (!done) {
        auto frame_iter = find_if(cat_scene.drawables.begin(), cat_scene.drawables.end(),
                                [](const Scene::Drawable & elem) { return elem.transform->name.find("Mouth") != std::string::npos; });
        if (frame_iter == cat_scene.drawables.end()) {
            done = true;
        } else {
            // printf("removing %s from cat_scene and into player_held_items\n", frame_iter->transform->name.c_str());
            player_held_items.push_back(*frame_iter);
            cat_scene.drawables.erase(frame_iter);
        }
    }

    // animate walking
    if (prev_player_position.z == player.transform_middle->position.z) { // potentially walking
        if (player.swatting) {
            player_swat.animate(cat_scene, true, elapsed);
        } else if (moved) {
            player_walking.animate(cat_scene, true, elapsed);
        } else {
            RemoveAllFrames(cat_scene);
            AddFrame(cat_scene, player_walking.frames[player_walking.frame_idx]);
        }
    } else if (prev_player_position.z < player.transform_middle->position.z) { // up jump
        player_up_jump.animate(cat_scene, true, elapsed);
    } else { // down jump
        player_down_jump.animate(cat_scene, true, elapsed);
    }

    // if object is being held
    if (player.holding) {
        // find frame being animated
        auto cat_itr = cat_scene.drawables.begin();
        if (cat_itr == cat_scene.drawables.end()) {
            printf("Error no cat animation frame exists!\n");
        } else {
            // printf("player is holding: %s\n", player.held_obj->transform->name.c_str());
            // printf("cat animation is: %s\n", cat_itr->transform->name.c_str());
            // parse out every including and past . in the name
            size_t period_pos = 0;
            //SOURCE: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
            std::string parsed_name = player.held_obj->transform->name;
            if ((period_pos = player.held_obj->transform->name.find(".")) != std::string::npos) {
                parsed_name = player.held_obj->transform->name.substr(0, period_pos);;
            }
            // printf("parsed_name is: %s\n", parsed_name.c_str());

            if (cat_itr->transform->name.find("Swat") == std::string::npos) { // is not a swat
                player.mouth->position = mouth_pos[cat_itr->transform->name];
                for (auto item = player_held_items.begin() ; item != player_held_items.end(); item++) {
                    // printf("loop: %s\n", item->transform->name.c_str());
                    if (item->transform->name == parsed_name + " Mouth") {
                        // printf("display: %s\n", item->transform->name.c_str());
                        AddFrame(cat_scene, *item);
                        player_held_items.erase(item);
                        break;
                    }
                }
            }
        }
    }
    // get name of animation frame if frame isn't swat
    // set position of Mouth to the value for that frame
    // make held item => scale 1.0, other scale 0.f
    
    { // camera position
        glm::vec3 camera_center = player.transform_middle->position;
        glm::vec3 camera_direction = glm::vec3(
            cos(phi + M_PI/2) * sin(theta),
            sin(phi + M_PI/2) * sin(theta),
            cos(theta));
        // paritally inspired by https://stackoverflow.com/questions/8812073/ray-and-square-rectangle-intersection-in-3d
        auto rect_distance = [camera_center, camera_direction](glm::vec3 P0, glm::vec3 P1, glm::vec3 P2) {
            glm::vec3 N = glm::cross(P1 - P0, P2 - P0);
            float t = glm::dot(P0 - camera_center, N) / glm::dot(camera_direction, N);
            glm::vec3 P = camera_center + t * camera_direction;
            if (t < 0) return std::numeric_limits<float>::infinity();

            float P_proj1 = glm::dot(P - P0, P1 - P0) / glm::length(P1 - P0);
            float P_proj2 = glm::dot(P - P0, P2 - P0) / glm::length(P2 - P0);

            if (0 <= P_proj1 && P_proj1 <= glm::length(P1 - P0)
            &&  0 <= P_proj2 && P_proj2 <= glm::length(P2 - P0)) {
                return t;
            } else {
                return std::numeric_limits<float>::infinity();
            }
        };
        auto bbox_distance = [rect_distance](glm::vec3* bbox) {
            float min_dist = std::numeric_limits<float>::infinity();
            // top
            min_dist = std::min(min_dist, rect_distance(bbox[2], bbox[1], bbox[6]));
            // bottom
            min_dist = std::min(min_dist, rect_distance(bbox[3], bbox[7], bbox[0]));
            // left
            min_dist = std::min(min_dist, rect_distance(bbox[5], bbox[4], bbox[6]));
            // right
            min_dist = std::min(min_dist, rect_distance(bbox[1], bbox[2], bbox[0]));
            // front
            min_dist = std::min(min_dist, rect_distance(bbox[2], bbox[6], bbox[3]));
            // back
            min_dist = std::min(min_dist, rect_distance(bbox[1], bbox[0], bbox[5]));
            return min_dist;
        };
        // TODO: remove this once the camera is no longer a child of the cat
        player.camera->transform->parent = nullptr;

        float radius = camera_radius;
        // loop  through all drawables in each scene
        for (auto room_type : current_rooms) {
            switch_rooms(room_type);
            for (auto &drawable : current_scene->drawables) {
                 if (drawable.transform->name == "Magazine Collided"
                ||  drawable.transform->name == "Mug Collided")
                    continue;
                radius = std::min(radius, 0.99f * bbox_distance(drawable.transform->bbox));
            }
        }
        // for (auto &drawable : living_room_scene.drawables) {
        //     // if (bbox_distance(drawable.transform->bbox) < radius) {
        //     //     std::cout << drawable.transform->name << std::endl;
        //     // }
        //     // manually skip some items with weird bbox's
        //     if (drawable.transform->name == "Magazine Collided"
        //     ||  drawable.transform->name == "Mug Collided")
        //         continue;
        //     radius = std::min(radius, 0.99f * bbox_distance(drawable.transform->bbox));
        // }
        // for (auto &drawable : kitchen_scene.drawables) {
        //     radius = std::min(radius, 0.99f * bbox_distance(drawable.transform->bbox));
        // }

        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        player.camera->transform->position = camera_center + radius * camera_direction;
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
    swat.pressed = false;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
    // Draw scene meshes
    {
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
    }

    // Draw text
    game_text.edit_state(game_text.SCORE, std::to_string(score));
    game_text.edit_state(game_text.TIME, game_timer.to_string());

    if (display_collide) game_text.edit_state(game_text.COLLISION, collide_label);
    else                 game_text.edit_state(game_text.COLLISION, " ");
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
    for (auto room_type : current_rooms) {
        switch_rooms(room_type);
        current_scene->draw(*player.camera);
    }
    // wdfs_scene.draw(*player.camera);
    // living_room_scene.draw(*player.camera);
    // kitchen_scene.draw(*player.camera);

    // { // DISPLAY BOUNDING BOXES FOR DEBUG PURPOSES!!!!!
        // glDisable(GL_DEPTH_TEST);
        // DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));

        // for (auto room_type : current_rooms) {
        //     switch_rooms(room_type);
        
        //     for (auto obj : (*current_objects)) {
        //         if (obj.collision_type != CollisionType::PushOff && obj.collision_type != CollisionType::Steal) continue;
        //         auto tip = obj.capsule.tip;
        //         auto base = obj.capsule.base;
        //         auto r = obj.capsule.radius;

        //         // tip
        //         glm::vec3 tip_center = glm::vec3(tip.x, tip.y, tip.z - r);
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(0.f, 0.f, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(0.f, 0.f, r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(0.f, r, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(0.f, -r, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(r, 0.f, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(tip_center, tip_center + glm::vec3(-r, 0.f, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));

        //         // base
        //         glm::vec3 base_center = glm::vec3(base.x, base.y, base.z + r);
        //         draw_lines.draw(base_center, base_center + glm::vec3(0.f, 0.f, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(base_center, base_center + glm::vec3(0.f, 0.f, r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(base_center, base_center + glm::vec3(0.f, r, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(base_center, base_center + glm::vec3(0.f, -r, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(base_center, base_center + glm::vec3(r, 0.f, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //         draw_lines.draw(base_center, base_center + glm::vec3(-r, 0.f, 0.f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        //     }
        // }

        // draw_lines.draw(center_, tri_point_, glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // float r = player.radius;

        // glm::vec3 capsule0_tip = player.transform_front->make_local_to_world() * glm::vec4(player.transform_front->position, 1.0f);
        // capsule0_tip.z += 1.0f;
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip, glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip + glm::vec3(0.f, 0.f, -2*r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip + glm::vec3(0.f, r, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip + glm::vec3(0.f, -r, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip + glm::vec3(r, 0.f, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
        // draw_lines.draw(capsule0_tip + glm::vec3(0.f, 0.f, -r), capsule0_tip + glm::vec3(-r, 0.f, -r), glm::u8vec4(0xff, 0xff, 0x00, 0xff));

        // glm::vec3 capsule1_tip = player.transform_middle->position;
        // capsule1_tip.z += 1.0f;
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip, glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip + glm::vec3(0.f, 0.f, -2*r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip + glm::vec3(0.f, r, -r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip + glm::vec3(0.f, -r, -r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip + glm::vec3(r, 0.f, -r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(capsule1_tip + glm::vec3(0.f, 0.f, -r), capsule1_tip + glm::vec3(-r, 0.f, -r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        // //head bump
        // glm::vec3 head_bump = player.head_bump->make_local_to_world() * glm::vec4(player.head_bump->position, 1.0f);
        // draw_lines.draw(head_bump, head_bump + glm::vec3(0.f, 0.f, -r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(head_bump, head_bump + glm::vec3(0.f, 0.f, r), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(head_bump, head_bump + glm::vec3(0.f, r, 0.f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(head_bump, head_bump + glm::vec3(0.f, -r, 0.f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(head_bump, head_bump + glm::vec3(r, 0.f, 0.f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        // draw_lines.draw(head_bump, head_bump + glm::vec3(-r, 0.f, 0.f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        // for (auto &drawable : (*current_scene).drawables) {
        //     // continue;
        //     if (drawable.transform->name != "Key") continue;

        //     // draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     // draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

        //     // draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
        //     // draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0xff, 0xff, 0xff));


        //     // top
        //     draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        //     // // bottom
        //     draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

        //     // left
        //     draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[7], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        //     // right
        //     draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[0], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        //     // front
        //     draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[2], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        //     // back
        //     draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[4], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
        //     draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0x00, 0xff, 0x00, 0xff));

        // }

    // }

	{ //overlay some text:
        glDisable(GL_DEPTH_TEST);
        game_text.update_state();
        game_text.draw_text(game_text.LEFT_X - 20.0f, game_text.TOP_Y + 20.0f, glm::vec3(0.1f, 0.1f, 0.1f));
    }
}
