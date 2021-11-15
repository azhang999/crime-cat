/*
    ######################################################################## 
                                Collision Handling
    ######################################################################## 
    
    Most of these utility routines were adapted directly using the tutorial located
    by the blog of Wicked Engine, an open-source C++ engine: 
    https://wickedengine.net/2020/04/26/capsule-collision-detection/

*/

#include "RoomObject.hpp"
#include <glm/glm.hpp>
#include <iostream>

glm::vec3 closest_point_on_line_segment(glm::vec3 A, glm::vec3 B, glm::vec3 Point);

bool sphere_triangle_collision(glm::vec3 center, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
                                glm::vec3 *pen_normal, float *pen_depth);

bool parallel_capsule_triangle_collision(glm::vec3 A, glm::vec3 B, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float radius, 
                                         glm::vec3 *pen_normal, float *pen_depth);

bool capsule_triangle_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
                                glm::vec3 *pen_normal, float *pen_depth);

bool capsule_rectagle_collision(glm::vec3 tip, glm::vec3 base, float radius, 
                                glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, 
                                glm::vec3 *pen_normal, float *pen_depth);

bool capsule_bbox_collision(glm::vec3 tip, glm::vec3 base, float radius, glm::vec3 *p, 
                                SurfaceType *surface, glm::vec3 *pen_normal, float *pen_depth);

bool capsule_capsule_collision(float a_radius, glm::vec3 a_tip, glm::vec3 a_base, 
                               float b_radius, glm::vec3 b_tip, glm::vec3 b_base);