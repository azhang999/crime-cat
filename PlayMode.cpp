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
            float diameter = std::max(std::abs(drawable.transform->bbox[5].x - drawable.transform->bbox[1].x), 
                                      std::abs(drawable.transform->bbox[2].y - drawable.transform->bbox[1].y));
            float radius = diameter / 2;
            float height = std::abs(drawable.transform->bbox[5].z - drawable.transform->bbox[7].z);
            glm::vec3 tip  = drawable.transform->position;
            glm::vec3 base = drawable.transform->position;
            tip.z  += height;

            // ###################################################### 
            // For collection COLLISION objects, set CollisionType 
            // ######################################################
            CollisionType type = CollisionType::None;
            if (drawable.transform->name == "Vase" || drawable.transform->name == "Mug") type = CollisionType::PushOff;
            else if (drawable.transform->name == "Key")                                  type = CollisionType::Swat;

            objects.push_back( RoomObject(drawable.transform->name, drawable.transform, drawable.transform->bbox, type, radius, height, tip, base));

            if (drawable.transform->name == "Rug") rug_height = tip.z;
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
    std::cout << "Start height: " << vase_obj.start_height << std::endl;
    std::cout << "End height: " << vase_obj.end_height << std::endl;


    // std::cout << "\n***** Looping through objects ****" << std::endl;
    // for (auto obj: objects) {
    //     if (!(obj.name->)) continue;
    //     std::cout << obj.name << " scale: " << glm::to_string(obj.transform->scale) << std::endl;

    //     std::cout << "Radius: " << obj.capsule.radius << std::endl;
    //     std::cout << "Tip: " << glm::to_string(obj.capsule.tip) << std::endl;
    //     std::cout << "Base: " << glm::to_string(obj.capsule.base) << std::endl;
    // }
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

// COLLISION CHECKING: TODO: refactor into another file
// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
glm::vec3 closest_point_on_line_segment(glm::vec3 A, glm::vec3 B, glm::vec3 Point) {
    glm::vec3 AB = B - A;
    float t = glm::dot(Point - A, AB) / glm::dot(AB, AB);
    return A + std::min(std::max(t, 0.f), 1.f) * AB;
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool sphere_triangle_collision(glm::vec3 center, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
                                glm::vec3 *pen_normal, float *pen_depth) {
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
        glm::vec3 best_point = point0;
        glm::vec3 intersection_vec;
        
        if (inside) {
            intersection_vec = center - point0;
        } else {
            float distsq;

            glm::vec3 d = center - point1;
            float best_distsq = dot(d, d);
            best_point = point1;
            intersection_vec = d;
        
            d = center - point2;
            distsq = dot(d, d);
            if (distsq < best_distsq) {
                best_distsq = distsq;
                best_point = point2;
                intersection_vec = d;
            }
        
            d = center - point3;
            distsq = glm::dot(d, d);
            if (distsq < best_distsq) {
                best_distsq = distsq;
                best_point = point3; 
                intersection_vec = d;
            }
        }
        
        float len = glm::length(intersection_vec);
        *pen_normal = glm::normalize(intersection_vec);
        *pen_depth = radius - len;
        return true;
    }

    return false;
}

bool parallel_capsule_triangle_collision(glm::vec3 A, glm::vec3 B, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float radius, 
                                         glm::vec3 *pen_normal, float *pen_depth) {
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

    return sphere_triangle_collision(center, radius, p0, p1, p2, pen_normal, pen_depth);
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
// Note for triangle normal, p0, p1, p2 matters
bool capsule_triangle_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
                                glm::vec3 *pen_normal, float *pen_depth) {
    // Compute capsule line endpoints A, B like before in capsule-capsule case:
    glm::vec3 CapsuleNormal = glm::normalize(tip - base); 
    glm::vec3 LineEndOffset = CapsuleNormal * radius; 
    glm::vec3 A = base + LineEndOffset; 
    glm::vec3 B = tip - LineEndOffset;
    
    // Then for each triangle, ray-plane intersection:
    //  N is the triangle plane normal (it was computed in sphere - triangle intersection case)
    glm::vec3 N = glm::normalize(glm::cross(p1 - p0, p2 - p0)); // plane normal

    if (std::abs(glm::dot(N, CapsuleNormal)) == 0.f) {
        return parallel_capsule_triangle_collision(A, B, p0, p1, p2, radius, pen_normal, pen_depth);
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

    return sphere_triangle_collision(center, radius, p0, p1, p2, pen_normal, pen_depth);
}

bool capsule_rectagle_collision(glm::vec3 tip, glm::vec3 base, float radius, 
                                glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, 
                                glm::vec3 *pen_normal, float *pen_depth) {
    if (capsule_triangle_collision(tip, base, radius, p0, p3, p1, pen_normal, pen_depth)) {
        return true;
    }

    return capsule_triangle_collision(tip, base, radius, p2, p1, p3, pen_normal, pen_depth);
}

bool capsule_bbox_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 *p, 
                                SurfaceType *surface, glm::vec3 *pen_normal, float *pen_depth) {
    // top
    if (capsule_rectagle_collision(tip, base, radius, p[6], p[5], p[1], p[2], pen_normal, pen_depth)) {
        *surface = TOP;
        return true;
    }

    // bottom
    if (capsule_rectagle_collision(tip, base, radius, p[4], p[7], p[3], p[0], pen_normal, pen_depth)) {
        *surface = BOT;
        return true;
    }

    // left
    if (capsule_rectagle_collision(tip, base, radius, p[4], p[5], p[6], p[7], pen_normal, pen_depth)) {
        *surface = LEFT;
        return true;
    }

    // right
    if (capsule_rectagle_collision(tip, base, radius, p[3], p[2], p[1], p[0], pen_normal, pen_depth)) {
        *surface = RIGHT;
        return true;
    }

    // front
    if (capsule_rectagle_collision(tip, base, radius, p[7], p[6], p[2], p[3], pen_normal, pen_depth)) {
        *surface = FRONT;
        return true;
    }

    // back
    if (capsule_rectagle_collision(tip, base, radius, p[0], p[1], p[5], p[4], pen_normal, pen_depth)) {
        *surface = BACK;
        return true;
    }

    return false;
}

// SOURCE: https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool capsule_capsule_collision(float a_radius, glm::vec3 a_tip, glm::vec3 a_base, 
                               float b_radius, glm::vec3 b_tip, glm::vec3 b_base) {
    // Capsule A
    glm::vec3 a_Normal = glm::normalize(a_tip - a_base);
    glm::vec3 a_LineEndOffset = a_Normal * a_radius;
    glm::vec3 a_A = a_base + a_LineEndOffset;
    glm::vec3 a_B = a_tip  - a_LineEndOffset;

    // Capsule B
    glm::vec3 b_Normal = glm::normalize(b_tip - b_base);
    glm::vec3 b_LineEndOffset = b_Normal * b_radius;
    glm::vec3 b_A = b_base + b_LineEndOffset;
    glm::vec3 b_B = b_tip  - b_LineEndOffset;

    // Vectors between line endpoints
    glm::vec3 v0 = b_A - a_A;
    glm::vec3 v1 = b_B - a_A; 
    glm::vec3 v2 = b_A - a_B; 
    glm::vec3 v3 = b_B - a_B;
    // Squared distances
    float d0 = glm::dot(v0, v0); 
    float d1 = glm::dot(v1, v1); 
    float d2 = glm::dot(v2, v2); 
    float d3 = glm::dot(v3, v3);

    // Select the best potential endpoint on capsule A:
    glm::vec3 pot_endpoint;
    if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1) {
        pot_endpoint = a_B;
    } else {
        pot_endpoint = a_A;
    }

    // Select point on capsule B line segment nearest to best potential endpoint on A capsule
    glm::vec3 best_B = closest_point_on_line_segment(b_A, b_B, pot_endpoint);
    glm::vec3 best_A = closest_point_on_line_segment(a_A, a_B, best_B); 

    // Perform sphere intersection routine
    glm::vec3 penetration_normal = best_A - best_B;
    float len = glm::length(penetration_normal);
    penetration_normal = glm::normalize(penetration_normal);
    float penetration_depth = a_radius + b_radius - len;
    
    return (penetration_depth > 0);
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

    // std::string object_collide_name = collide(&collision_obj);
    std::string object_collide_name = collide();

    auto collision_obj_iter = find_if(objects.begin(), objects.end(),
                                        [object_collide_name](const RoomObject &elem) { return elem.name == object_collide_name; });
    RoomObject &collision_obj = *(collision_obj_iter);

    if (player.swatting && object_collide_name == "Key") {    // erase on swat
        std::cout << "+++++++ SWATTING Key +++++++" << std::endl;

        // erase key on swat
        auto key_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == "Key"; });
        scene.drawables.erase(key_iter);

        score += 3;

        player.swatting = false;
    } 
    if (object_collide_name == "Vase" && !collision_obj.done) {
        // Save object's original position
        collision_obj.prev_position = collision_obj.transform->position;

        // Calculate player's displacement in this timestep
        glm::vec3 offset = (player.transform->position - prev_player_position);

        // Get object's actual drawable and move it
        auto obj_drawable_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                        [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == object_collide_name; });
        auto &obj_drawable = *(obj_drawable_iter);

        obj_drawable.transform->position += offset;
        for (auto i = 0; i < 8; i++) {
            collision_obj.orig_bbox[i] = obj_drawable.transform->bbox[i]; // Save original BBOX position
            obj_drawable.transform->bbox[i] += offset;                    // Update to new
        }

        // ############## Test for collisions against other objects ##############
        if (capsule_collide(collision_obj, &collision_obj.pen_dir, &collision_obj.pen_depth) == "") {    // Move capsule in absence of collisions
            collision_obj.capsule.tip  = collision_obj.capsule.tip  + (collision_obj.transform->position - collision_obj.prev_position);
            collision_obj.capsule.base = collision_obj.capsule.base + (collision_obj.transform->position - collision_obj.prev_position);

            // ------- Get sidetable top object for table-item location detection --------
            auto sidetable_iter = find_if(scene.drawables.begin(), scene.drawables.end(),
                                    [object_collide_name](const Scene::Drawable & elem) { return elem.transform->name == "SideTable"; });
            auto sidetable = *(sidetable_iter);

            if (!collision_obj.collided && 
                !((sidetable.transform->bbox[1].x <= obj_drawable.transform->bbox[5].x && obj_drawable.transform->bbox[1].x <= sidetable.transform->bbox[5].x)
                && (sidetable.transform->bbox[1].y <= obj_drawable.transform->bbox[2].y && obj_drawable.transform->bbox[1].y <= sidetable.transform->bbox[2].y))) {
                collision_obj.is_falling = true;
            }
        }
        else {  // ------- COLLISION OCCURED -------
            // Slide-alone code inspired from https://wickedengine.net/2020/04/26/capsule-collision-detection/, "Usage"
            // TODO: Assumes objects are off the same weight - can add per-object weights (i.e. heavier objects = slower velocities)
            glm::vec3 obj_velocity = offset / elapsed; //(collision_obj.transform->position - collision_obj.prev_position) / elapsed;
            float obj_velocity_length = glm::length(obj_velocity);

            std::cout << "MOVEMENT VELOCITY: " << glm::to_string(obj_velocity) << ", normalized = " << glm::to_string(glm::normalize(obj_velocity)) << ", length = " << obj_velocity_length << std::endl;

            obj_velocity = glm::normalize(obj_velocity);

            if (!glm::isnan(obj_velocity.x) && !glm::isnan(obj_velocity.y) && !glm::isnan(obj_velocity.z)) {

                // Fix velocity
                glm::vec3 undesired_motion = collision_obj.pen_dir * glm::dot(collision_obj.pen_dir, obj_velocity);
                glm::vec3 desired_motion = obj_velocity - undesired_motion;
                obj_velocity = obj_velocity_length * desired_motion;
                std::cout << "DESIRED MOTION: " << glm::to_string(desired_motion) << std::endl;
                std::cout << "REVISED VELOCITY: " << glm::to_string(obj_velocity) << std::endl;

                // Undo movement 
                // obj_drawable.transform->position = vase_orig;
                // for (auto i = 0; i < 8; i++) {
                //     obj_drawable.transform->bbox[i] = orig_vase_bbox[i];
                // }

                // Re-do object movement with displacement vector calculated using this new velocity
                glm::vec3 displacement = obj_velocity * elapsed;
                obj_drawable.transform->position = collision_obj.prev_position + displacement;
                for (auto i = 0; i < 8; i++) {
                    obj_drawable.transform->bbox[i] = collision_obj.orig_bbox[i] + displacement;
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

    // Handle vase falling off of table
    auto vase_obj_iter = find_if(objects.begin(), objects.end(),
                                        [](const RoomObject & elem) { return elem.name == "Vase"; });
    auto &vase_obj = *(vase_obj_iter);

    if (!vase_obj.done && vase_obj.is_falling) {

        vase_obj.air_time += elapsed;

        float height = vase_obj.start_height + 0.5f * gravity * vase_obj.air_time * vase_obj.air_time;
        if (height <= vase_obj.end_height) {

            vase_obj.transform->position.z = vase_obj.end_height;
            vase_obj.capsule.base.z = vase_obj.end_height;
            vase_obj.capsule.tip.z = vase_obj.end_height + vase_obj.capsule.height;

            vase_obj.is_falling = false;
            vase_obj.air_time = 0.0f;

            // float diff = vase_obj.start_height - vase_obj.end_height;
            // std::cout << "DIFF = " << diff << std::endl;
            // std::cout << "current bottom: " << vase_obj.transform->bbox[5].z << ", top: " << vase_obj.transform->bbox[0].z << std::endl;
            // for (auto i = 0; i < 8; i++) {
            //     vase_obj.transform->bbox[i].z = vase_obj.orig_bbox[i].z - diff;
            // }
            // std::cout << "updated bottom: " << vase_obj.transform->bbox[5].z << ", top: " << vase_obj.transform->bbox[0].z << std::endl;
            vase_obj.transform->bbox[5].z = vase_obj.end_height + vase_obj.capsule.height;
            vase_obj.transform->bbox[1].z = vase_obj.end_height + vase_obj.capsule.height;
            vase_obj.transform->bbox[2].z = vase_obj.end_height + vase_obj.capsule.height;
            vase_obj.transform->bbox[6].z = vase_obj.end_height + vase_obj.capsule.height;

            vase_obj.transform->bbox[0].z = vase_obj.end_height;
            vase_obj.transform->bbox[3].z = vase_obj.end_height;
            vase_obj.transform->bbox[4].z = vase_obj.end_height;
            vase_obj.transform->bbox[7].z = vase_obj.end_height;

            score += 5;
            vase_obj.collided = true;  // prevents user from gaining more points
            vase_obj.done = true;
        }
        else {
            vase_obj.transform->position.z = height;
            vase_obj.capsule.base.z = height;
            vase_obj.capsule.tip.z = height + vase_obj.capsule.height;

            // for (auto i = 0; i < 8; i++) {      // Don't forget to update vase bbox
            //     // std::cout << "new z: " << (0.5f * gravity * vase_obj.air_time * vase_obj.air_time) << std::endl;
            //     vase_obj.transform->bbox[i].z = height; //0.5f * gravity * vase_obj.air_time * vase_obj.air_time;
            // }
            vase_obj.transform->bbox[5].z = height + vase_obj.capsule.height;
            vase_obj.transform->bbox[1].z = height + vase_obj.capsule.height;
            vase_obj.transform->bbox[2].z = height + vase_obj.capsule.height;
            vase_obj.transform->bbox[6].z = height + vase_obj.capsule.height;

            vase_obj.transform->bbox[0].z = height;
            vase_obj.transform->bbox[3].z = height;
            vase_obj.transform->bbox[4].z = height;
            vase_obj.transform->bbox[7].z = height;
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
        //     if (obj.name != "Vase") continue;
            // auto vase_obj_iter = find_if(objects.begin(), objects.end(),
            //                                 [](const RoomObject & elem) { return elem.name == "Vase"; });
            // auto obj = *(vase_obj_iter);
            // auto tip = obj.capsule.tip;
            // auto base = obj.capsule.base;
            // auto radius = obj.capsule.radius;

            // // std::cout << "Penetration direction: " << glm::to_string(obj.pen_dir) << ", depth: " << obj.pen_depth << std::endl;

            // // tip
            // auto A = glm::vec3(tip.x + radius, tip.y + radius, tip.z);
            // auto B = glm::vec3(tip.x - radius, tip.y - radius, tip.z);
            // auto C = glm::vec3(tip.x + radius, tip.y - radius, tip.z);
            // auto D = glm::vec3(tip.x - radius, tip.y + radius, tip.z);
            
            // draw_lines.draw(A, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(B, C, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(D, B, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(A, D, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

            // // base
            // auto E = glm::vec3(base.x + radius, base.y + radius, base.z);
            // auto F = glm::vec3(base.x - radius, base.y - radius, base.z);
            // auto G = glm::vec3(base.x + radius, base.y - radius, base.z);
            // auto H = glm::vec3(base.x - radius, base.y + radius, base.z);
            
            // draw_lines.draw(E, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(F, G, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(H, F, glm::u8vec4(0x00, 0x00, 0xff, 0xff));
            // draw_lines.draw(E, H, glm::u8vec4(0x00, 0x00, 0xff, 0xff));

            // // sides
            // draw_lines.draw(A,E, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            // draw_lines.draw(B,F, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            // draw_lines.draw(C,G, glm::u8vec4(0x00, 0x00, 0x00, 0xff));
            // draw_lines.draw(D,H, glm::u8vec4(0x00, 0x00, 0x00, 0xff));


            // -------- penetration vector --------
        // }

        for (auto &drawable : scene.drawables) {
            if (drawable.transform->name != "Vase" && drawable.transform->name != "SideTable") continue;

            
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
