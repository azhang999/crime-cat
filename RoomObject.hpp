
#pragma once
#include "Scene.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include <glm/glm.hpp>

enum SurfaceType {TOP, BOT, FRONT, BACK, LEFT, RIGHT};

enum CollisionType {
	None,
	Steal,
	KnockOver,
	PushOff,
	Destroy,
};

class RoomObject {
	public:
		RoomObject() {};		// empty for when we just want to save a reference

		RoomObject(Scene::Transform *transform_, CollisionType collision_type_) : 
					transform(transform_), collision_type(collision_type_){
			this->name = this->transform->name;
			
            this->capsule.radius = std::max(std::abs(this->transform->bbox[5].x - this->transform->bbox[1].x), 
                                            std::abs(this->transform->bbox[2].y - this->transform->bbox[1].y)) / 2;
            this->capsule.height = std::abs(this->transform->bbox[5].z - this->transform->bbox[7].z);
            this->capsule.tip  = this->transform->position;
            this->capsule.base = this->transform->position;
            this->capsule.tip.z  += this->capsule.height/2;
            this->capsule.base.z  -= this->capsule.height/2;

			for (auto i = 0; i < 8; i++) {
				this->orig_bbox[i] = this->transform->bbox[i];
			}
            this->orig_pos = transform->position;
            this->orig_rotation = transform->rotation;
		}

		// ----- Transform properties -----
		std::string name;
		// std::string label;
		Scene::Transform *transform = nullptr;
		glm::vec3 orig_bbox[8];
        glm::vec3 orig_pos;
		glm::vec3 orig_scale;
        glm::quat orig_rotation;
		glm::vec3 prev_position;

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
		bool collided = false;
		bool done = false;
		glm::vec3 pen_dir = glm::vec3(0);
		float pen_depth = 0.f;
	
		// ----- Collision resolution -----
		std::vector<Scene::Drawable> reaction_drawables;
		bool has_sound = false;
		std::vector<Load<Sound::Sample> *> samples;

		// ***** Falling objects *****
		bool is_falling = false;
		float air_time = 0.0f;
		float start_height = 0.0f;
		float end_height   = 0.0f;
		float x_min = 0, x_max = 0, y_min = 0, y_max = 0;

        bool is_moving = false;
        glm::vec3 move_dir = glm::vec3(0.f);
        float speed = 0.0f;
        float given_speed = 0.f;
        bool spin = false;
};