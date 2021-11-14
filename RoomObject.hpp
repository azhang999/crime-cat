
#pragma once
#include "Scene.hpp"
#include <glm/glm.hpp>

enum SurfaceType {TOP, BOT, FRONT, BACK, LEFT, RIGHT};

enum CollisionType {
	None,
	Swat,
	KnockOver,
	PushOff,
};

class RoomObject {
	public:
		RoomObject() {};		// empty for when we just want to save a reference

		RoomObject(Scene::Transform *transform_, CollisionType collision_type_) : 
					transform(transform_), collision_type(collision_type_) {
			this->name = this->transform->name;
			
            this->capsule.radius = std::max(std::abs(this->transform->bbox[5].x - this->transform->bbox[1].x), 
                                            std::abs(this->transform->bbox[2].y - this->transform->bbox[1].y)) / 2;
            this->capsule.height = std::abs(this->transform->bbox[5].z - this->transform->bbox[7].z);
            this->capsule.tip  = this->transform->position;
            this->capsule.base = this->transform->position;
            this->capsule.tip.z  += this->capsule.height;

			for (auto i = 0; i < 8; i++) {
				this->orig_bbox[i] = this->transform->bbox[i];
			}
		}

		// ----- Transform properties -----
		std::string name;
		Scene::Transform *transform = nullptr;
		glm::vec3 orig_bbox[8];

		// ----- Capsule properties -----
		struct Capsule {
			float radius = 0.5f;
			float height = 1.0f;
			glm::vec3 tip;		 // tip position
			glm::vec3 base; 	 // base position
		};
		Capsule capsule;
	
		// ----- Collision detection properties -----
		CollisionType collision_type = None;
		bool collided = false;					 // vase_was_pushed
		bool done = false;						 // vase_done
		glm::vec3 pen_dir = glm::vec3(0);
		float pen_depth = 0.f;
	
		// ----- Collision resolution -----
		glm::vec3 prev_position;	 			 // vase_orig

		// ***** Falling objects *****
		bool is_falling = false;				 // vase_is_falling
		float air_time = 0.0f;					 // vase_air_time
		float start_height = 0.0f;				 // vase_starting_height
		float end_height   = 0.0f;				 // rug_height
		float x_min = 0, x_max = 0, y_min = 0, y_max = 0;

};