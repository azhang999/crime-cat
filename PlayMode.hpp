#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Mesh.hpp"
#include "Load.hpp"

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

    void GenerateBBox(Scene &scene, Load<MeshBuffer> &meshes);
	void updateBBox(Scene::Transform *transform, glm::vec3 displacement);
    std::string collide();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, swat;

	//local copy of the game scene (so code can change it during gameplay):
	Scene cat_scene;
    Scene living_room_scene;
    Scene kitchen_scene;

    //entire scene
    //Scene full_scene;

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

    struct Animation {
        std::vector<Scene::Drawable> frames;
        std::vector<float> frame_times;
        uint32_t frame_idx = 0;
        float timer = 0.f;
        std::string name;

        void animate(Scene &scene, bool enable, float elapsed);
    } player_walking, player_up_jump, player_down_jump;

    // struct Walking {
    //     std::vector<Scene::Drawable> frames;
    //     std::vector<float> frame_times = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    //     uint32_t frame_idx = 0;
    //     float timer = 0.f;
    // } player_walking;

	int score = 0;

	float theta = 0;
	float phi = ((float)M_PI)/2.f;
	float camera_radius = 10.0f;

	Scene::Transform *vase_transform = nullptr;
	bool vase_is_falling = false;
	float vase_air_time = 0.0f;
	float vase_starting_height;
	glm::vec3 orig_vase_bbox[8];
	bool vase_was_pushed = false;

    Scene::Transform *wall1, *wall2, *wall3, *wall4;

};
