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

// void PlayMode::AttachToGround(Scene::Transform *transform) {
//     printf("Calling AttachToGround\n");
//     printf("1\n");
//     player.ground = transform;
//     if (transform->top_stand) {
//         printf("2\n");
//         player.surface = TOP;
//         printf("3\n");
//         player.transform->position.z = transform->bbox[2].z+ 1.0f;
//         printf("4\n");
//     } else if (transform->bot_stand) {
//         player.surface = BOT;
//         player.transform->position.z = transform->bbox[3].z+ 1.0f;
//     } else if (transform->front_stand) {
//         player.surface = FRONT;
//         player.transform->position.z = transform->bbox[3].z+ 1.0f;
//     } else if (transform->back_stand) {
//         player.surface = BACK;
//         player.transform->position.z = transform->bbox[0].z+ 1.0f;
//     } else if (transform->left_stand) {
//         player.surface = LEFT;
//         player.transform->position.z = transform->bbox[7].z+ 1.0f;
//     } else if (transform->right_stand) {
//         player.surface = RIGHT;
//         player.transform->position.z = transform->bbox[2].z+ 1.0f;
//     } else {
//         std::cout << transform->name << " does not have a standable surface " << std::endl;
//     }

//     player.ground_level = player.transform->position.z;
// }



PlayMode::PlayMode() : scene(*living_room_scene) {

    // get player transform - TODO don't loop through again
    // for (auto &drawable : scene.drawables) {
    //     if (drawable.transform->name == "Player") {
    //         player.transform = drawable.transform;
    //         player.tip = player.transform->position;
    //         player.tip.z += 1.0f;
    //         player.base = player.transform->position;
    //         player.base.z -= 1.0f;
    //     }
    // }



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

        // std::cout << drawable.transform->name << std::endl;
        // std::cout << bbox.P1[0] << " " << bbox.P1[1] << " " << bbox.P1[2] << std::endl;
        // std::cout << bbox.P2[0] << " " << bbox.P2[1] << " " << bbox.P2[2] << std::endl;
        // std::cout << bbox.P3[0] << " " << bbox.P3[1] << " " << bbox.P3[2] << std::endl;
        // std::cout << bbox.P4[0] << " " << bbox.P4[1] << " " << bbox.P4[2] << std::endl;
        // std::cout << bbox.P5[0] << " " << bbox.P5[1] << " " << bbox.P5[2] << std::endl;
        // std::cout << bbox.P6[0] << " " << bbox.P6[1] << " " << bbox.P6[2] << std::endl;
        // std::cout << bbox.P7[0] << " " << bbox.P7[1] << " " << bbox.P7[2] << std::endl;
        // std::cout << bbox.P8[0] << " " << bbox.P8[1] << " " << bbox.P8[2] << std::endl;
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
        //else if (drawable.transform->name == "Floor") {
        //     player.ground = drawable.transform;
        //     AttachToGround(drawable.transform);
        // }

        // --------------------------------------------------
        if (drawable.transform->name != "Player") {
            // std::cout << drawable.transform->name << ": ";
            auto scale = drawable.transform->scale;

            float radius = (std::max(scale.x, scale.y) + 0.25) / 2;
            float half_height = scale.z / 2;

            glm::vec3 tip  = drawable.transform->position;
            glm::vec3 base = drawable.transform->position;
            tip.z  += half_height;
            base.z -= half_height;

            objects.push_back(
                RoomObject(drawable.transform->name, drawable.transform, drawable.transform->bbox, drawable.transform->position.z,
                           radius, tip, base, CollisionType::Swat)
            );

            // std::cout << "\t" << glm::to_string(drawable.transform->scale) << std::endl;
        }
        // --------------------------------------------------
	}

    // std::cout << "\n***** Looping through objects ****" << std::endl;
    for (auto obj: objects) {
        std::cout << obj.name << " scale: " << glm::to_string(obj.transform->scale) << std::endl;

        std::cout << "Radius: " << obj.capsule.radius << std::endl;
        std::cout << "Tip: " << glm::to_string(obj.capsule.tip) << std::endl;
        std::cout << "Base: " << glm::to_string(obj.capsule.base) << std::endl;

        std::cout << "" << std::endl;
    }



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
			//camera looks down -z (basically at the player's feet) when pitch is at zero.
			// pitch = std::min(pitch, 0.95f * 3.1415926f);
			// pitch = std::max(pitch, 0.05f * 3.1415926f);
			// player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

			return true;
		}
	}

	return false;
}

// COLLISION CHECKING: TODO: refactor into another file
// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
glm::vec3 closest_point_on_line_segment(glm::vec3 A, glm::vec3 B, glm::vec3 Point) {
    glm::vec3 AB = B - A;
    float t = glm::dot(Point - A, AB) / glm::dot(AB, AB);
    return A + std::min(std::max(t, 0.f), 1.f) * AB;
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool sphere_triangle_collision(glm::vec3 center, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
    // float3 p0, p1, p2; // triangle corners
    // float3 center; // sphere center
    glm::vec3 N = glm::normalize(glm::cross(p1 - p0, p2 - p0)); // plane normal
    float dist = glm::dot(center - p0, N); // signed distance between sphere and plane
    // if (!mesh.is_double_sided() && dist > 0) continue; // can pass through back side of triangle (optional)
    if (dist < -radius || dist > radius) {
        return false; // no intersection
    } 
    glm::vec3 point0 = center - N * dist; // projected sphere center on triangle plane
    
    // Now determine whether point0 is inside all triangle edges: 
    glm::vec3 c0 = glm::cross(point0 - p0, p1 - p0);
    glm::vec3 c1 = glm::cross(point0 - p1, p2 - p1);
    glm::vec3 c2 = glm::cross(point0 - p2, p0 - p2);
    bool inside = (glm::dot(c0, N) <= 0) && (glm::dot(c1, N) <= 0) && (glm::dot(c2, N) <= 0);
    
    float radiussq = radius * radius; // sphere radius squared
    
    // Edge 1:
    glm::vec3 point1 = closest_point_on_line_segment(p0, p1, center);
    glm::vec3 v1 = center - point1;
    float distsq1 = glm::dot(v1, v1);
    bool intersects = distsq1 < radiussq;
    
    // Edge 2:
    glm::vec3 point2 = closest_point_on_line_segment(p1, p2, center);
    glm::vec3 v2 = center - point2;
    float distsq2 = glm::dot(v2, v2);
    intersects |= distsq2 < radiussq;
    
    // Edge 3:
    glm::vec3 point3 = closest_point_on_line_segment(p2, p0, center);
    glm::vec3 v3 = center - point3;
    float distsq3 = glm::dot(v3, v3);
    intersects |= distsq3 < radiussq;

    if (inside || intersects) {
        // float3 best_point = point0;
        // float3 intersection_vec;
        
        // if (inside) {
        //     intersection_vec = center - point0;
        // } else {
        //     float3 d = center - point1;
        //     float best_distsq = dot(d, d);
        //     best_point = point1;
        //     intersection_vec = d;
        
        //     d = center - point2;
        //     float distsq = dot(d, d);
        //     if (distsq < best_distsq) {
        //         distsq = best_distsq;
        //         best_point = point2;
        //         intersection_vec = d;
        //     }
        
        //     d = center - point3;
        //     float distsq = dot(d, d);
        //     if (distsq < best_distsq) {
        //         distsq = best_distsq;
        //         best_point = point3; 
        //         intersection_vec = d;
        //     }
        // }
        
        // float3 len = length(intersection_vec);  // vector3 length calculation: sqrt(dot(v, v))
        // float3 penetration_normal = penetration_vec / len;  // normalize
        // float penetration_depth = radius - len; // radius = sphere radius
        return true; // intersection success
    }

    return false;
}

bool parallel_capsule_triangle_collision(glm::vec3 A, glm::vec3 B, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float radius) {
    glm::vec3 close0 = closest_point_on_line_segment(A, B, p0);
    glm::vec3 close1 = closest_point_on_line_segment(A, B, p1);
    glm::vec3 close2 = closest_point_on_line_segment(A, B, p2);

    glm::vec3 center = close0;
    float d = glm::distance(close0, p0);

    float temp_d = glm::distance(close1, p1);
    if (temp_d < d) {
        center = close1;
        d = temp_d;
    }

    temp_d = glm::distance(close2, p2);
    if (temp_d < d) {
        center = close2;
        d = temp_d;
    }

    return sphere_triangle_collision(center, radius, p0, p1, p2);
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
// Note for triangle normal, p0, p1, p2 matters
bool capsule_triangle_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
    // Compute capsule line endpoints A, B like before in capsule-capsule case:
    glm::vec3 CapsuleNormal = glm::normalize(tip - base); 
    glm::vec3 LineEndOffset = CapsuleNormal * radius; 
    glm::vec3 A = base + LineEndOffset; 
    glm::vec3 B = tip - LineEndOffset;
    
    // Then for each triangle, ray-plane intersection:
    //  N is the triangle plane normal (it was computed in sphere - triangle intersection case)
    glm::vec3 N = glm::normalize(glm::cross(p1 - p0, p2 - p0)); // plane normal

    if (std::abs(glm::dot(N, CapsuleNormal)) == 0.f) {
        return parallel_capsule_triangle_collision(A, B, p0, p1, p2, radius);
    }


    float t = glm::dot(N, (p0 - base) / std::abs(glm::dot(N, CapsuleNormal))); // this is producing nans since glm::dot(N, CapsuleNormal) is returning 0
    
    glm::vec3 line_plane_intersection = base + CapsuleNormal * t;

    
    glm::vec3 reference_point = glm::vec3(0.f);// {find closest point on triangle to line_plane_intersection};

    // Determine whether line_plane_intersection is inside all triangle edges: 
    glm::vec3 c0 = glm::cross(line_plane_intersection - p0, p1 - p0);
    glm::vec3 c1 = glm::cross(line_plane_intersection - p1, p2 - p1);
    glm::vec3 c2 = glm::cross(line_plane_intersection - p2, p0 - p2);
    bool inside = (glm::dot(c0, N) <= 0) && (glm::dot(c1, N) <= 0) && (glm::dot(c2, N) <= 0);

    if (inside) {
        reference_point = line_plane_intersection;
    } else {
        // Edge 1:
        glm::vec3 point1 = closest_point_on_line_segment(p0, p1, line_plane_intersection);
        glm::vec3 v1 = line_plane_intersection - point1;
        float distsq = glm::dot(v1, v1);
        float best_dist = distsq;
        reference_point = point1;
        
        // Edge 2:
        glm::vec3 point2 = closest_point_on_line_segment(p1, p2, line_plane_intersection);
        glm::vec3 v2 = line_plane_intersection - point2;
        distsq = glm::dot(v2, v2);
        if (distsq < best_dist) {
            reference_point = point2;
            best_dist = distsq;
        }
        
        // Edge 3:
        glm::vec3 point3 = closest_point_on_line_segment(p2, p0, line_plane_intersection);
        glm::vec3 v3 = line_plane_intersection - point3;
        distsq = glm::dot(v3, v3);
        if (distsq < best_dist) {
            reference_point = point3;
            best_dist = distsq;
        }
    }
    
    // The center of the best sphere candidate:
    glm::vec3 center = closest_point_on_line_segment(A, B, reference_point);

    return sphere_triangle_collision(center, radius, p0, p1, p2);
}

bool capsule_rectagle_collision(glm::vec3 tip, glm::vec3 base, float radius, 
                                glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    if (capsule_triangle_collision(tip, base, radius, p0, p3, p1)) {
        return true;
    }

    return capsule_triangle_collision(tip, base, radius, p2, p1, p3);
}

bool capsule_bbox_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 *p, SurfaceType *surface) {
    // top
    if (capsule_rectagle_collision(tip, base, radius, p[6], p[5], p[1], p[2])) {
        *surface = TOP;
        return true;
    }

    // bottom
    if (capsule_rectagle_collision(tip, base, radius, p[4], p[7], p[3], p[0])) {
        *surface = BOT;
        return true;
    }

    // left
    if (capsule_rectagle_collision(tip, base, radius, p[4], p[5], p[6], p[7])) {
        *surface = LEFT;
        return true;
    }

    // right
    if (capsule_rectagle_collision(tip, base, radius, p[3], p[2], p[1], p[0])) {
        *surface = RIGHT;
        return true;
    }

    // front
    if (capsule_rectagle_collision(tip, base, radius, p[7], p[6], p[2], p[3])) {
        *surface = FRONT;
        return true;
    }

    // back
    if (capsule_rectagle_collision(tip, base, radius, p[0], p[1], p[5], p[4])) {
        *surface = BACK;
        return true;
    }

    return false;
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool capsule_capsule_collision() {
    return true;
}

std::string PlayMode::collide() {
    for (auto &drawable : scene.drawables) {
        if (drawable.transform->name == "Rug") continue; // Rug is kinda blocky
        if (drawable.transform->name == "Player") continue;
        if (drawable.transform->name == "Facing") continue;
        if (drawable.transform->name == "CatPlayer") continue;

        SurfaceType surface;
        if (capsule_bbox_collision(player.tip, player.base, player.radius, drawable.transform->bbox, &surface)) {
            return drawable.transform->name;
        }
    }

    SurfaceType surface;
    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall1->bbox, &surface)) {
        return wall1->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall2->bbox, &surface)) {
        return wall2->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall3->bbox, &surface)) {
        return wall3->name;
    }

    if (capsule_bbox_collision(player.tip, player.base, player.radius, wall4->bbox, &surface)) {
        return wall4->name;
    }

    return "";
}

void PlayMode::update(float elapsed) {
    glm::vec3 prev_player_position = player.transform->position;
    
	//move player:
    //combine inputs into a move:
    {
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

    std::string object_collide_name = collide();

    if (player.swatting && object_collide_name == "Key") {    // erase on swat
        std::cout << "+++++++ SWATTING Key +++++++" << std::endl;

        // erase key on swat
        auto key_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == "Key"; });
        scene.drawables.erase(key_iter);

        score += 3;

        player.swatting = false;
    } 
    if (object_collide_name == "Vase") {    // erase on swat
        std::cout << "+++++++ PUSHING Vase +++++++" << std::endl;

        // Get vase object & move it
        auto obj_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == object_collide_name; });
        auto obj = *(obj_iter);
        obj.transform->position += (player.transform->position - prev_player_position);
        for (auto i = 0; i < 8; i++) {
                obj.transform->bbox[i] += (player.transform->position - prev_player_position);
        }

        // Get sidetable top object for table-item location detection
        auto sidetable_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == "SideTable"; });
        auto sidetable = *(sidetable_iter);

        // Check if vase fell off the edge of the table
        // std::cout << "x: " << sidetable.transform->bbox[5].x << "," << sidetable.transform->bbox[1].x << std::endl;
        // std::cout << "y: " << sidetable.transform->bbox[2].y << "," << sidetable.transform->bbox[1].y << std::endl;
        // std::cout << "vase pos: " << glm::to_string(sidetable.transform->position) << std::endl;

        if (!vase_was_pushed &&
            !((sidetable.transform->bbox[1].x <= obj.transform->position.x && obj.transform->position.x <= sidetable.transform->bbox[5].x)
           && (sidetable.transform->bbox[1].y <= obj.transform->position.y && obj.transform->position.y <= sidetable.transform->bbox[2].y))) {
            // std::cout << "VASE IS FALLING" << std::endl;
            vase_is_falling = true;
            vase_starting_height = obj.transform->position.z;
            vase_transform = obj.transform;
            for (auto i = 0; i < 8; i++) {
                orig_vase_bbox[i].z = obj.transform->bbox[i].z;
            }
        }
    } 


    // else if (object_collide_name == "Vase" || object_collide_name == "Key") { //drawable.transform->name == "Key") {
    //     std::cout << "$$$$$$ PUSHING $$$$$$" << std::endl;

    //     auto obj_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
    //                             [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == object_collide_name; });
        
    //     auto obj = *(obj_iter);

    //     obj.transform->position += (player.transform->position - prev_player_position);
    //     for (auto i = 0; i < 8; i++) {
    //             obj.transform->bbox[i] += (player.transform->position - prev_player_position);
    //     }
    // } 
    else if (object_collide_name != "") { // undo movement
        player.transform->position = prev_player_position;
        player.tip = prev_player_position;
        player.tip.z += 1.0f;
        player.base = prev_player_position;
        player.base.z -= 1.0f;
    }

    prev_player_position = player.transform->position;


    // // handle jumping and gravity
    // if (player.jumping) {
    //     player.air_time += elapsed;
    //     if (player.air_time > 2.0f) {
    //         player.air_time = 0.f;
    //         player.jumping = false;
    //     } else {
    //         player.transform->position.z += 5.0f * elapsed;
    //     }
    // }


    //  // gravity
    // player.transform->position.z -= 2.0f * elapsed;

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

    // Handle vase falling off of table
    if (vase_is_falling) {
        std::cout << "pushing vase off..." << std::endl;

        vase_air_time += elapsed;

        float height = vase_starting_height + 0.5f * gravity * vase_air_time * vase_air_time;
        std::cout << height << std::endl;
        if (height <= 1.0f) {
            vase_transform->position.z = 1.0f;
            vase_is_falling = false;
            vase_air_time = 0.0f;

            // // Add this margin back to bounding box coordinates
            // float margin = 1.0f - height;
            // for (auto i = 0; i < 8; i++) {
            //     vase_transform->bbox[i].z += margin;
            // }
            float diff = vase_starting_height - 1.0f;
            for (auto i = 0; i < 8; i++) {
                vase_transform->bbox[i].z = orig_vase_bbox[i].z - diff;
            } 
            score += 5;
            vase_was_pushed = true;  // prevents user from gaining more points
        }
        else {
            vase_transform->position.z = height;
            for (auto i = 0; i < 8; i++) {      // Don't forget to update vase bbox
                vase_transform->bbox[i].z = 0.5f * gravity * vase_air_time * vase_air_time;
            }
        }
    }


    // float player_starting_height = drawable.transform->bbox[5].z + 1.0f;
    //             float height = player_starting_height + 0.5f * gravity * player.air_time * player.air_time;
    //             std::cout << height << std::endl;
    //             if (height <= player.ground_level) {
    //                 player.transform->position.z = player.ground_level;
    //                 player.on_table = false;
    //                 player.air_time = 0.0f;
    //             }
    //             else {
    //                 player.transform->position.z = height;
    //             }



    // // check for collisions:
    // {
    //     for (auto &drawable : scene.drawables) {
    //         if (drawable.transform->name == "Floor") continue;
    //         if (drawable.transform->name == "Rug") continue; // Rug is kinda blocky
    //         if (drawable.transform->name == "Player") continue;
    //         if (drawable.transform->name == "Facing") continue;

    //         SurfaceType surface;
    //         if (capsule_bbox_collision(player.tip, player.base, player.radius, drawable.transform->bbox, &surface)) {

    //             if (player.swatting && drawable.transform->name == "Key") {    // erase on swat
    //                 std::cout << "+++++++ SWATTING +++++++" << std::endl;

    //                 // erase key on swat
    //                 auto key_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
    //                                         [&drawable](const Scene::Drawable & elem) { return &elem == &drawable; });
    //                 scene.drawables.erase(key_iter);

    //                 player.swatting = false;
    //             }

    //             if (drawable.transform->name == "Vase" || drawable.transform->name == "Key") { //drawable.transform->name == "Key") {
    //                 std::cout << "$$$$$$ PUSHING $$$$$$" << std::endl;
    //                 drawable.transform->position += (player.transform->position - prev_player_position);
    //                 for (auto i = 0; i < 8; i++) {
    //                      drawable.transform->bbox[i] += (player.transform->position - prev_player_position);
    //                 }
    //                 return;     // don't want to reset position
    //             }
                
    //             if (drawable.transform->name == "Table.005" && surface == TOP) {
    //                 if (!player.on_table) {
    //                     std::cout << "now he is on the table" << std::endl;
    //                     player.on_table = true;
    //                 }
    //             }

    //             player.transform->position = prev_player_position;

                // std::cout << drawable.transform->name << std::endl;
                // switch (surface) {
                //     case TOP: {
                //         std::cout << "TOP" << std::endl;
                //         break;
                //     }
                //     case BOT: {
                //         std::cout << "BOT" << std::endl;
                //         break;
                //     }
                //     case FRONT: {
                //         std::cout << "FRONT" << std::endl;
                //         break;
                //     }
                //     case BACK: {
                //         std::cout << "BACK" << std::endl;
                //         break;
                //     }
                //     case LEFT: {
                //         std::cout << "LEFT" << std::endl;
                //         break;
                //     }
                //     case RIGHT: {
                //         std::cout << "RIGHT" << std::endl;
                //         break;
                //     }
                // }
            // }
            // else {
            //     if (drawable.transform->name == "Table.005" && player.on_table) {
            //         if (!((drawable.transform->bbox[1].x <= player.transform->position.x && player.transform->position.x <= drawable.transform->bbox[5].x)
            //            && (drawable.transform->bbox[1].y <= player.transform->position.y && player.transform->position.y <= drawable.transform->bbox[2].y))) {

            //             std::cout << "*** WALK OFF TABLE, player position = " << glm::to_string(player.transform->position) << std::endl;
                        
            //             player.air_time += elapsed;
            //             float player_starting_height = drawable.transform->bbox[5].z + 1.0f;
            //             float height = player_starting_height + 0.5f * gravity * player.air_time * player.air_time;
            //             std::cout << height << std::endl;
            //             if (height <= player.ground_level) {
            //                 player.transform->position.z = player.ground_level;
            //                 player.on_table = false;
            //                 player.air_time = 0.0f;
            //             }
            //             else {
            //                 player.transform->position.z = height;
            //             }
            //         }
            //         // player.transform->position.z = player.ground_level;
            //     }
            // }            
    //     }
    // }

    // player.tip = player.transform->position;
    // player.tip.z += 1.0f;
    // player.base = player.transform->position;
    // player.base.z -= 1.0f;




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

    // { // DISPLAY BOUNDING BOXES FOR DEBUG PURPOSES!!!!!
    //     glDisable(GL_DEPTH_TEST);
    //     DrawLines draw_lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));

    //     for (auto &drawable : scene.drawables) {
    //         if (drawable.transform->name != "SideTable" && drawable.transform->name != "Vase") continue;

    //         //top
    //         draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[2], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[6], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[6], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // bottom
    //         draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[0], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[4], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

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
    //         draw_lines.draw(drawable.transform->bbox[2], drawable.transform->bbox[3], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[3], drawable.transform->bbox[7], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[7], drawable.transform->bbox[6], glm::u8vec4(0xff, 0x00, 0x00, 0xff));

    //         // back
    //         draw_lines.draw(drawable.transform->bbox[5], drawable.transform->bbox[1], glm::u8vec4(0xff, 0x00, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[1], drawable.transform->bbox[0], glm::u8vec4(0x00, 0xff, 0x00, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[0], drawable.transform->bbox[4], glm::u8vec4(0x00, 0x00, 0xff, 0xff));
    //         draw_lines.draw(drawable.transform->bbox[4], drawable.transform->bbox[5], glm::u8vec4(0xff, 0x00, 0x00, 0xff));


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
