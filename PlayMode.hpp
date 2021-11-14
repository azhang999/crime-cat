#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <iostream>


enum SurfaceType {TOP, BOT, FRONT, BACK, LEFT, RIGHT};

enum CollisionType {
	None,
	Swat,
	KnockOver,
	PushOff,
};

const float DIST_EPS = 0.01f;
class RoomObject {
	public:
		RoomObject() {};		// empty for when we just want to save a reference

		RoomObject(std::string name_, Scene::Transform *transform_, glm::vec3 orig_bbox_[8], CollisionType collision_type_, 
				float radius_, float height_, glm::vec3 tip_, glm::vec3 base_) : 
			name(name_), transform(transform_), collision_type(collision_type_) {
			
			for (auto i = 0; i < 8; i++) {
				this->orig_bbox[i] = orig_bbox_[i];
			}

			this->capsule.radius = radius_;
			this->capsule.height = height_;
			this->capsule.tip 	 = tip_;
			this->capsule.base 	 = base_;
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
		glm::vec3 pen_dir;
		float pen_depth;
	
		// ----- Collision resolution -----
		glm::vec3 prev_position;	 			 // vase_orig

		// ***** Falling objects *****
		bool is_falling = false;				 // vase_is_falling
		float air_time = 0.0f;					 // vase_air_time
		float start_height = 0.0f;				 // vase_starting_height
		float end_height   = 0.0f;				 // rug_height
		float x_min = 0, x_max = 0, y_min = 0, y_max = 0;

};


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    void AttachToGround(Scene::Transform *transform);
	void updateBBox(Scene::Transform *transform, glm::vec3 displacement);
    std::string collide();
	std::string capsule_collide(RoomObject &current_obj, glm::vec3 *pen_normal, float *pen_depth);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, swat;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

    float gravity = -15.0f;

    struct Player {
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
        Scene::Transform *facing = nullptr;

        float radius = 0.5f;
        glm::vec3 tip;
        glm::vec3 base;

		//camera is at player's head
		Scene::Camera *camera = nullptr;

        //jumping
        float init_up_v = 8.0f;
        float air_time = 0.0f;
        bool jumping = false;
		bool swatting = false;
		bool on_table = false;

        float starting_height;

        // Scene::Transform *ground = nullptr;
        // SurfaceType surface = TOP;
        // float ground_level = 0.f;
	} player;

	std::vector<RoomObject> objects;

	int score = 0;

    Scene::Transform *wall1, *wall2, *wall3, *wall4;
};
