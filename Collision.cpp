#include "Collision.hpp"


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

bool is_almost_up_vec(glm::vec3 &v) {
    glm::vec3 n_v = glm::normalize(v);

    float epsilon = 0.01f;
    if (n_v.x >= epsilon || n_v.x <= -epsilon) {
        return false;
    }

    if (n_v.y >= epsilon || n_v.y <= -epsilon) {
        return false;
    }

    if (n_v.z >= epsilon + 1.0f || n_v.z <= -epsilon - 1.0f) {
        return false;
    }

    return true;
}

bool capsule_rectagle_collision(glm::vec3 tip, glm::vec3 base, float radius, 
                                glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, 
                                glm::vec3 *pen_normal, float *pen_depth) {

    glm::vec3 pen_normal1; float pen_depth1;
    glm::vec3 pen_normal2; float pen_depth2;
    bool collide1 = capsule_triangle_collision(tip, base, radius, p0, p3, p1, &pen_normal1, &pen_depth1);
    bool collide2 = capsule_triangle_collision(tip, base, radius, p2, p1, p3, &pen_normal2, &pen_depth2);

    if (collide1 && collide2) {
        bool use_up1 = is_almost_up_vec(pen_normal1);
        if (use_up1) {
            *pen_normal = pen_normal1;
            *pen_depth = pen_depth1;
        } else {
            *pen_normal = pen_normal2;
            *pen_depth = pen_depth2;
        }
    } else if (collide1) {
        *pen_normal = pen_normal1;
        *pen_depth = pen_depth1;
    } else if (collide2) {
        *pen_normal = pen_normal2;
        *pen_depth = pen_depth2;
    }
    return collide1 || collide2;
}

bool capsule_bbox_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 *p, 
                                SurfaceType *surface, glm::vec3 *pen_normal, float *pen_depth) {
    
    // first check the standable surface
    // if (standing_surface == TOP) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[6], p[5], p[1], p[2], pen_normal, pen_depth)) {
    //         *surface = TOP;
    //         return true;
    //     }
    // } else if (standing_surface == BOT) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[4], p[7], p[3], p[0], pen_normal, pen_depth)) {
    //         *surface = BOT;
    //         return true;
    //     }
    // } else if (standing_surface == LEFT) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[4], p[5], p[6], p[7], pen_normal, pen_depth)) {
    //         *surface = LEFT;
    //         return true;
    //     }
    // } else if (standing_surface == RIGHT) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[3], p[2], p[1], p[0], pen_normal, pen_depth)) {
    //         *surface = RIGHT;
    //         return true;
    //     }
    // } else if (standing_surface == FRONT) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[7], p[6], p[2], p[3], pen_normal, pen_depth)) {
    //         *surface = FRONT;
    //         return true;
    //     }
    // } else if (standing_surface == BACK) {
    //     if (capsule_rectagle_collision(tip, base, radius, p[0], p[1], p[5], p[4], pen_normal, pen_depth)) {
    //         *surface = BACK;
    //         return true;
    //     }
    // }
    
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