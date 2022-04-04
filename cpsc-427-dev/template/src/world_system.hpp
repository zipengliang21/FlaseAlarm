#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>


#include "render_system.hpp"
#include "world_init.hpp"

#include "GameLevel/LevelManager.h"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	// Level Manager, all the logic is in it
	std::unique_ptr<LevelManager> levelManager;

	WorldSystem();

	// Releases all associated resources
	~WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void key_callback(int key, int scancode, int action, int mod);
	void mouse_move_callback(double x,double y);
	void mouse_button_callback(int button, int action, int mods);

	// OpenGL window handle
	GLFWwindow* window;
	bool isFullScreen;

	

	RenderSystem* renderer;

	//float next_eagle_spawn;

	//Entity camera;
	//Entity light;
	//// C++ random number generator
	//std::default_random_engine rng;
	//std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
