#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "world_init.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void mouse_button_callback(int button, int action, int mods);

	// get texture id
	TEXTURE_ASSET_ID getTextureIDOfLevelButton(int level);

	// Interpolate
	float approach(float goal_v, float cur_v, float dt);

	// restart level
	void restart_game();
	

	// change level
	int changeLevel(std::string buttonAction);

	// display content for different pages
	void showLevelContent(int level);
	void showTutorial();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of bug eaten by the chicken, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	float next_eagle_spawn;
	float next_bug_spawn;
	Entity player_student;
	Entity guard;
	Entity exit;
	GameState* gameState = NULL;
	Entity camera;
	Entity light;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* chicken_dead_sound;
	Mix_Chunk* chicken_eat_sound;
	Mix_Chunk* wall_collision_sound;
	Mix_Chunk* fire_alarm_sound;
	Mix_Chunk* trap_sound;


	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
