#include "Mode.hpp"

#include "Scene.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include "RoomObject.hpp"
#include "Collision.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "GameText.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <functional>
#include <iostream>
#include <limits>


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	enum RoomType {
		None,
		LivingRoom,
		Kitchen,
		Bathroom,
        Office,
        Bedroom,
        WallsDoorsFloorsStairs
	};

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    void GenerateBBox(Scene &scene, Load<MeshBuffer> &meshes);
	void updateBBox(Scene::Transform *transform, glm::vec3 displacement);

    void generate_wdfs_objects(Scene &scene, std::vector<RoomObject> &objects);
	void generate_living_room_objects(Scene &scene, std::vector<RoomObject> &objects);
    void generate_kitchen_objects(Scene &scene, std::vector<RoomObject> &objects);
    void generate_bedroom_objects(Scene &scene, std::vector<RoomObject> &objects);
    void generate_bathroom_objects(Scene &scene, std::vector<RoomObject> &objects);
    void generate_office_objects(Scene &scene, std::vector<RoomObject> &objects);
    void generate_room_objects(Scene &scene, std::vector<RoomObject> &objects, RoomType room_type);
	void switch_rooms(RoomType room_type);
	float get_surface_below_height(float &closest_dist);
	// void check_room();
	// std::string floor_collide(); //RoomType floor_collide();

    Scene::Transform *collide();
    std::string paw_collide();
	std::string capsule_collide(RoomObject &current_obj, glm::vec3 *pen_normal, float *pen_depth);
    void interact_with_objects(float elapsed, std::string object_collide_name, glm::vec3 player_motion);

	// When the game is first loaded, it's after showng the instruction screen
	// But the instruction screen can be brought back up
    std::shared_ptr< Mode > instruct_mode;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, swat;

	// ------------------ Rooms ------------------
	// RoomType current_room = RoomType::LivingRoom;
	Scene *current_scene = nullptr;
	std::vector<RoomObject> *current_objects = nullptr;

	//local copy of the game scene (so code can change it during gameplay):
	Scene cat_scene;
    Scene living_room_scene;
    Scene kitchen_scene;
    Scene wdfs_scene;
    Scene bedroom_scene;
    Scene bathroom_scene;
    Scene office_scene;

	std::vector<RoomObject> living_room_objects;
	std::vector<RoomObject> kitchen_objects;
    std::vector<RoomObject> wdfs_objects;
    std::vector<RoomObject> bedroom_objects;
    std::vector<RoomObject> bathroom_objects;
    std::vector<RoomObject> office_objects;

    // hardcode all rooms in for now
    std::vector<RoomType> current_rooms = {
        WallsDoorsFloorsStairs, 
        LivingRoom, 
        Kitchen, 
        Bedroom, 
        Bathroom,
        Office
    };

	// save floors of all rooms specially for collisions to avoid lookups
	Scene::Transform *living_room_floor = nullptr;
	Scene::Transform *kitchen_floor = nullptr;

	Scene::Transform *counter_transform = nullptr; // lol we have no walls atm

    //entire scene
    //Scene full_scene;

    float gravity = -15.0f;

    struct Player {
		//transform is at player's feet and will be yawed by mouse left/right motion:
        Scene::Transform *transform_front = nullptr;
		Scene::Transform *transform_middle = nullptr;
        Scene::Transform *paw = nullptr;
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
        float swatting_timer = 0.f;
		bool on_table = false;

        float starting_height;

        // Scene::Transform *ground = nullptr;
        // SurfaceType surface = TOP;
        // float ground_level = 0.f;

		void update_position(glm::vec3 new_pos) {
			// Update mesh position
			transform_middle->position = new_pos;

			// Update capsule
			transform_middle->position = new_pos;
			tip = new_pos;
			tip.z += 1.0f;
			base = new_pos;
			base.z -= 1.0f;
		}
	} player;

	struct Shadow {
		Scene::Drawable *drawable;
		float closest_dist = 0;

		void update_position(glm::vec3 new_pos, float height, float dist) {
			drawable->transform->position = new_pos;
			drawable->transform->position.z = height + 0.001f;
			closest_dist = dist;
		}
	} shadow;

    glm::vec3 penetration_normal;
    float penetration_depth;

    int num_collide_objs = 0;
    // bool collide_front = false;
    // bool collide_middle = false;

    struct Animation {
        std::vector<Scene::Drawable> frames;
        std::vector<float> frame_times;
        uint32_t frame_idx = 0;
        float timer = 0.f;
        std::string name;

        void animate(Scene &scene, bool enable, float elapsed);
    } player_walking, player_up_jump, player_down_jump, player_swat;

	int score = 0;
	float theta = -0.3f * (float)M_PI;
	float phi = ((float)M_PI)/2.f;
	float camera_radius = 10.0f;

	struct GameTimer {
		float seconds = 1.0f * 60.f;		// TODO change back to 8min
		std::string to_string() {
			int sec = static_cast<int>(std::round(seconds));
			int minutes = sec / 60;
			int seconds = sec % 60;
			if (seconds < 10)
				return std::to_string(minutes) + ":0" + std::to_string(seconds);
			else 
				return std::to_string(minutes) + ":" + std::to_string(seconds);
		}
	};
	GameTimer game_timer;
    bool game_over = false;

    Scene::Transform *wall1, *wall2, *wall3, *wall4;

	std::string script_path = data_path("./text/play.txt");
    GameText game_text;

	std::string collide_label;
	bool display_collide = false;
	const float MAX_COL_TIME = 3.0f;
	float collide_msg_time = 3.0f;
};
