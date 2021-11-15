#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "RoomObject.hpp"
#include "Collision.hpp"
#include "Mesh.hpp"
#include "Load.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <iostream>


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	enum RoomType {
		None,
		LivingRoom,
		Kitchen,
		// Bathroom
	};

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    void GenerateBBox(Scene &scene, Load<MeshBuffer> &meshes);
	void updateBBox(Scene::Transform *transform, glm::vec3 displacement);

	void generate_room_objects(Scene &scene, std::vector<RoomObject> &objects, RoomType room_type);
	void switch_rooms(RoomType room_type);
	// void check_room();
	// std::string floor_collide(); //RoomType floor_collide();

    std::string collide();
	std::string capsule_collide(RoomObject &current_obj, glm::vec3 *pen_normal, float *pen_depth);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, swat;

	// ------------------ Rooms ------------------
	RoomType current_room = RoomType::LivingRoom;
	Scene *current_scene = nullptr;
	std::vector<RoomObject> *current_objects = nullptr;

	//local copy of the game scene (so code can change it during gameplay):
	Scene player_scene;
    Scene living_room_scene;
    Scene kitchen_scene;

	std::vector<RoomObject> living_room_objects;
	std::vector<RoomObject> kitchen_objects;

	// save floors of all rooms specially for collisions to avoid lookups
	Scene::Transform *living_room_floor = nullptr;
	Scene::Transform *kitchen_floor = nullptr;

	Scene::Transform *counter_transform = nullptr; // lol we have no walls atm

    //entire scene
    //Scene full_scene;

    float gravity = -15.0f;

    struct Player {
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
        // Scene::Transform *facing = nullptr;

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

	int score = 0;
	float theta = 0;
	float phi = ((float)M_PI)/2.f;
	float camera_radius = 10.0f;

    Scene::Transform *wall1, *wall2, *wall3, *wall4;
};
