// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_EAGLES = 15;
const size_t MAX_BUG = 5;
const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;
const float PLAYER_SPEED = 400;

// Background size
float bg_X = window_width_px * 1.5;
float bg_Y = window_height_px * 1.5;

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
	, next_eagle_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	if (wall_collision_sound != nullptr)
		Mix_FreeChunk(wall_collision_sound);
	if (fire_alarm_sound != nullptr)
		Mix_FreeChunk(fire_alarm_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Chicken Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
	wall_collision_sound = Mix_LoadWAV(audio_path("wall_collision.wav").c_str());
	fire_alarm_sound = Mix_LoadWAV(audio_path("fire_alarm.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr || wall_collision_sound == nullptr || fire_alarm_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str(),
			audio_path("wall_collision.wav").c_str(),
			audio_path("fire_alarm.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	//title_ss << "Points: " << points;
	title_ss << "Fire Alarm";

	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Spawning new bug
	next_bug_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.eatables.components.size() <= MAX_BUG && next_bug_spawn < 0.f) {
		// !!!  TODO A1: Create new bug with createBug({0,0}), as for the Eagles above
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the chicken state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity entity : registry.winTimers.entities) {
		WinTimer& counter = registry.winTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.winTimers.remove(entity);
			screen.greener_screen_factor = 0;
			restart_game();
			return true;
		}
	}

	// turn screen to grren
	screen.greener_screen_factor = 1 - min_counter_ms / 3000;


	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter
	for (Entity entity: registry.walkTimers.entities) {
		WalkTimer& counter = registry.walkTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			counter.counter_ms = 12000;
			Motion& motion = registry.motions.get(entity);
			motion.velocity = {-1*motion.velocity[0] , motion.velocity[1]};
		}
	}

	for (Entity entity : registry.rotateTimers.entities) {
		RotateTimer& counter = registry.rotateTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			counter.counter_ms = 3000;
			Motion& motion = registry.motions.get(entity);
			motion.velocity.x = -motion.velocity.x;
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset Camera
	renderer->translationMatrix = { {-0.5f, 0.f, 0.f}, {0.f, 1.0f, 0.f}, {0.f, 0.f, 0.f} };

	

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create a new chicken
	player_student = createStudent(renderer, { bg_X /2, bg_Y - 50 });

	// Create textbox
	auto student_pos = registry.motions.get(player_student).position;
	//createTextBox(renderer, { student_pos.x + 100, student_pos.y - 100 });


	const float WALL_SIZE = 20.2f;
	// Create a new exit
	exit = createExit(renderer, { bg_X - 100, 50 + WALL_SIZE });

	// Create light
	camera = createLight(renderer, { bg_X - WALL_SIZE - 65, bg_Y - WALL_SIZE - 35 });

	// Create a camera
	camera = createCamera(renderer, { bg_X - WALL_SIZE - 50, bg_Y - WALL_SIZE - 10 });

	// Create walls 
	float counter_X = 0;
	float counter_Y = 0;
	while (counter_X < bg_X) {
		createWall(renderer, { counter_X, counter_Y });
		counter_X += WALL_SIZE;
	}
	
	while (counter_Y < bg_Y) {
		createWall(renderer, { counter_X, counter_Y });
		counter_Y += WALL_SIZE;
	}

	while (counter_X > 0) {
		createWall(renderer, { counter_X, counter_Y });
		counter_X -= WALL_SIZE;
	}

	while (counter_Y > 0) {
		createWall(renderer, { counter_X, counter_Y });
		counter_Y -= WALL_SIZE;
	}

	float bl_X = bg_X * 3 / 8;
	float bl_Y = bg_Y * 5 / 8;
	while(bl_X > 0) {
		if (bl_X < (bg_X * 3 / 8 - 7 * WALL_SIZE) || bl_X > (bg_X * 3 / 8 - 3 * WALL_SIZE)) {
			createWall(renderer, { bl_X, bl_Y });
		}
		bl_X -= WALL_SIZE;
	}

	bl_X = bg_X * 3 / 8;
	while (bl_Y < bg_Y) {
		createWall(renderer, { bl_X, bl_Y });
		bl_Y += WALL_SIZE;
	}

	float ul_X = bg_X * 3 / 8;
	float ul_Y = bg_Y * 3 / 8;
	while (ul_X > 0) {
		createWall(renderer, { ul_X, ul_Y });
		ul_X -= WALL_SIZE;
	}

	ul_X = bg_X * 3 / 8;
	while (ul_Y > 0) {
		if (ul_Y < (bg_Y * 3 / 8 - 10 * WALL_SIZE) || ul_Y >(bg_Y * 3 / 8 - 6 * WALL_SIZE)) {
			createWall(renderer, { ul_X, ul_Y });
		}
		ul_Y -= WALL_SIZE;
	}

	float ur_X = bg_X * 5 / 8;
	float ur_Y = bg_Y * 3 / 8;
	while (ur_X < bg_X) {
		if (ur_X < (bg_X * 5 / 8 + 5 * WALL_SIZE) || ur_X >(bg_X * 5 / 8 + 9 * WALL_SIZE)) {
			createWall(renderer, { ur_X, ur_Y });
		}
		ur_X += WALL_SIZE;
	}

	ur_X = bg_X * 5 / 8;
	while (ur_Y > 0) {
		createWall(renderer, { ur_X, ur_Y });
		ur_Y -= WALL_SIZE;
	}

	float ub_X = bg_X * 5 / 8;
	float ub_Y = bg_Y * 5 / 8;
	while (ub_X < bg_X) {
		createWall(renderer, { ub_X, ub_Y });
		ub_X += WALL_SIZE;
	}

	ub_X = bg_X * 5 / 8;
	while (ub_Y < bg_Y) {
		if (ub_Y < (bg_Y * 5 / 8 + 4 * WALL_SIZE) || ub_Y >(bg_Y * 5 / 8 + 8 * WALL_SIZE)) {
			createWall(renderer, { ub_X, ub_Y });
		}
		ub_Y += WALL_SIZE;
	}

	registry.colors.insert(player_student, {1, 0.8f, 0.8f});

	// Create security guard, TODO: make it a list of guard
	guard = createGuard(renderer, vec2(bg_X - 100 , bg_Y /2));
	//registry.motions.get(guard).position = { window_width_px - 100 , window_height_px / 2 };
	//registry.motions.get(guard).velocity = { -100.f , 0 };
	// !! TODO A3: Enable static eggs on the ground
	// Create eggs on the floor for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
			         { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions; // TODO: @Tim, is the reference here needed?
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the chicken sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);

					// !!! TODO A1: change the chicken orientation and color on death
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					++points;

					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the chicken entity by modifying the ECS registry
				}
			}
			else if (registry.stopables.has(entity_other)) {
				if (!registry.stopeds.has(entity)) {
					registry.stopeds.emplace(entity);
				}
				vec2 velocity = registry.motions.get(entity).velocity;
				registry.motions.get(entity).velocity = { 0, 0 };
				vec2 position = registry.motions.get(entity).position;
				Mix_PlayChannel(-1, wall_collision_sound, 0);
				if (velocity.x > 0) {
					registry.motions.get(entity).position = { position.x - 30.f, position.y};
				}
				if (velocity.x < 0) {
					registry.motions.get(entity).position = { position.x + 30.f, position.y };
				}
				if (velocity.y > 0) {
					registry.motions.get(entity).position = { position.x, position.y - 30.f };
				}
				if (velocity.y < 0) {
					registry.motions.get(entity).position = { position.x, position.y + 30.f};
				}
				//registry.motions.get(entity).position = { position.x, position.y };
			}
			else if (registry.wins.has(entity_other)) {
				Mix_PlayChannel(-1, fire_alarm_sound, 5);
				++points;
				if (!registry.wins.has(entity)) {
					registry.wins.emplace(entity);
				}
				createTextBox(renderer, { bg_X / 2, bg_Y / 2 });
			}
			
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Motion& motion = registry.motions.get(player_student);

	mat3 currPosition = renderer->translationMatrix;

	if (registry.wins.has(player_student)) {
		return;
	}

	
	if (key == GLFW_KEY_W) {
		registry.stopeds.remove(player_student);
		if (action == GLFW_PRESS) {
				
			motion.velocity = { 0,-PLAYER_SPEED };

		}
		else if (action == GLFW_RELEASE) {
			motion.velocity = { 0,0 };
		}
	}
	if (key == GLFW_KEY_S) {
		registry.stopeds.remove(player_student);
		if (action == GLFW_PRESS) {
			motion.velocity = { 0,PLAYER_SPEED };
		}
		else if (action == GLFW_RELEASE) {
			motion.velocity = { 0,0 };
		}
	}
	if (key == GLFW_KEY_A) {
		registry.stopeds.remove(player_student);
		if (action == GLFW_PRESS) {
			motion.velocity = { -PLAYER_SPEED,0 };
		}
		else if (action == GLFW_RELEASE) {
			motion.velocity = { 0,0 };
		}
	}
	if (key == GLFW_KEY_D) {
		registry.stopeds.remove(player_student);
		if (action == GLFW_PRESS) {
			motion.velocity = { PLAYER_SPEED,0 };
		}
		else if (action == GLFW_RELEASE) {
			motion.velocity = { 0,0 };
		}
	}
	

	/// .----------------------------------------
	if (key == GLFW_KEY_UP) {
		if (action == GLFW_PRESS && currPosition[1].y > -0.950) {
			renderer->translationMatrix[1].y = currPosition[1].y -0.05;
		}
	}
	if (key == GLFW_KEY_DOWN) {
		if (action == GLFW_PRESS && currPosition[1].y <0.950) {
			renderer->translationMatrix[1].y = currPosition[1].y + 0.050;
		}
	}
	if (key == GLFW_KEY_RIGHT) {
		if (action == GLFW_PRESS && currPosition[0].x > -0.950) {
			renderer->translationMatrix[0].x = currPosition[0].x - 0.050;
		}
	}
	if (key == GLFW_KEY_LEFT) {
		if (action == GLFW_PRESS && currPosition[0].x < 0.950) {
			renderer->translationMatrix[0].x = currPosition[0].x + 0.05;
		}
	}

	

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	//// Debugging
	//if (key == GLFW_KEY_D) {
	//	if (action == GLFW_RELEASE)
	//		debugging.in_debug_mode = false;
	//	else
	//		debugging.in_debug_mode = true;
	//}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the chicken's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
}
