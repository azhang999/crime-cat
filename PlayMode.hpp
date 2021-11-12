#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

enum SurfaceType {TOP, BOT, FRONT, BACK, LEFT, RIGHT};

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


	// Every object in the scene has a type of collision
	typedef enum CollisionType_t {
		Swat = 0,		// default
		KnockOver = 1,
		PushOff = 2,
	} CollisionType;


	class RoomObject {
		public:
			RoomObject(std::string name_, Scene::Transform *transform_, glm::vec3 orig_bbox_[8], float starting_height_, 
					float radius_, glm::vec3 tip_, glm::vec3 base_,
					CollisionType collision_type_) : 
				name(name_), transform(transform_), starting_height(starting_height_), 
				/*radius(radius_), tip(tip_), base(base_),*/ collision_type(collision_type_) {
				
				for (auto i = 0; i < 8; i++) {
					this->orig_bbox[i] = orig_bbox_[i];
				}

				this->capsule.radius = radius_;
				this->capsule.tip 	 = tip_;
				this->capsule.base 	 = base_;

				// this->radius 		  = std::max();
				// this->tip 			  = transform->position + 1.0f;
				// this->base			  = transform->position - 1.0f;
				// this->starting_height = transform_->position.z;
			}

			// ----- Transform properties -----
			std::string name;
			Scene::Transform *transform = nullptr;
			glm::vec3 orig_bbox[8];
			float starting_height = 0;

			// ----- Capsule properties -----
			struct Capsule {
				float radius = 0.5f; // default
				glm::vec3 tip;		 // tip position
				glm::vec3 base; 	 // base position
			};
			Capsule capsule;
		
			// ----- Collision properties -----
			CollisionType collision_type = Swat;
		
			// helpful for time-variability
			bool collided = false;
			bool is_falling = false;
			float air_time = 0.0f;
	};
	std::vector<RoomObject> objects;


	int score = 0;

	Scene::Transform *vase_transform = nullptr;
	bool vase_is_falling = false;
	float vase_air_time = 0.0f;
	float vase_starting_height;
	glm::vec3 orig_vase_bbox[8];
	bool vase_was_pushed = false;

    Scene::Transform *wall1, *wall2, *wall3, *wall4;

};
