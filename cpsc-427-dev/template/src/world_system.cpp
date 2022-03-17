// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "physics_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

// Game configuration
const size_t MAX_EAGLES = 15;
const size_t MAX_BUG = 5;
const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;
const float PLAYER_SPEED = 200;

// Background size
float bg_X = window_width_px * 1.5;
float bg_Y = window_height_px * 1.5;

// global variable to remember cursor position
float cursorX;
float cursorY;


int bot_to_top = 0;
int left_to_right = 0;

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
	if (death_sound != nullptr)
		Mix_FreeChunk(death_sound);
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
GLFWwindow *WorldSystem::create_window() {
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
	window = glfwCreateWindow(window_width_px, window_height_px, "False Alarm", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}
	isFullScreen = false;
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwSetWindowPos(window, mode->width / 7, mode->height / 5);

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->mouse_button_callback( _0, _1, _2 ); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_button_redirect);

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
	death_sound = Mix_LoadWAV(audio_path("wall_collision.wav").c_str());
	fire_alarm_sound = Mix_LoadWAV(audio_path("fire_alarm.wav").c_str());
	trap_sound = Mix_LoadWAV(audio_path("trap.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr || death_sound == nullptr || fire_alarm_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str(),
			audio_path("wall_collision.wav").c_str(),
			audio_path("fire_alarm.wav").c_str(),
			audio_path("trap.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg) {
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

	// update player velocity
	GameState& gameState = registry.gameStates.get(gameStateEntity);
	if (gameState.state == GameState::GAME_STATE::LEVEL_SELECTED) {
		Motion& player_motion = registry.motions.get(player_student);
		Motion& guard_motion = registry.motions.get(guard);

		player_motion.velocity.x = approach(player_motion.velocityGoal.x, player_motion.velocity.x, elapsed_ms_since_last_update / 5);
		player_motion.velocity.y = approach(player_motion.velocityGoal.y, player_motion.velocity.y, elapsed_ms_since_last_update / 5);

		guard_motion.velocity.x = approach(guard_motion.velocityGoal.x, guard_motion.velocity.x, elapsed_ms_since_last_update / 10);
		guard_motion.velocity.y = approach(guard_motion.velocityGoal.y, guard_motion.velocity.y, elapsed_ms_since_last_update / 10);


		// Remove debug info from the last step
		while (registry.debugComponents.entities.size() > 0)
			registry.remove_all_components_of(registry.debugComponents.entities.back());

		// Removing out of screen entities
		auto& motions_registry = registry.motions;

		// Remove entities that leave the screen on the left side
		// Iterate backwards to be able to remove without unterfering with the next object to visit
		// (the containers exchange the last element with the current)
		for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
			Motion& motion = motions_registry.components[i];
			if (motion.position.x + abs(motion.scale.x) < 0.f) {
				if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
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
		ScreenState& screen = registry.screenStates.components[0];

		float min_counter_ms = 3000.f;
		for (Entity entity : registry.deathTimers.entities) {
			// progress timer
			DeathTimer& counter = registry.deathTimers.get(entity);
			counter.counter_ms -= elapsed_ms_since_last_update;
			if (counter.counter_ms < min_counter_ms) {
				min_counter_ms = counter.counter_ms;
			}

			// restart the game once the death timer expired
			if (counter.counter_ms < 0) {
				registry.deathTimers.remove(entity);
				screen.darken_screen_factor = 0;
				// go back to menu when restarting the game
				gameState.state = GameState::GAME_STATE::LEVEL_SELECTION;
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

		// get the guard instance
		auto& guardObj = registry.deadlys.get(guard);
		//auto &mo = registry.motions.get(guard);
		//printf("%f,%f\n", mo.position.x, mo.position.y);

		// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter
		for (Entity entity : registry.walkTimers.entities) {
			WalkTimer& counter = registry.walkTimers.get(entity);
			Motion& motion = registry.motions.get(entity);
			counter.counter_ms -= elapsed_ms_since_last_update;
			if (counter.counter_ms < 0) {
				counter.counter_ms = 12000;

				motion.velocityGoal = { -1 * motion.velocityGoal[0] , motion.velocityGoal[1] };

				// if this is guard
				if (entity == guard)
				{
					Character::Direction dir;
					if (abs(motion.velocityGoal.x) >= abs(motion.velocityGoal.y)) {
						if (motion.velocityGoal.x >= 0) // now the guard is moving right
							dir = Character::Direction::RIGHT;
						else
							dir = Character::Direction::LEFT;
					}
					else {
						if (motion.velocityGoal.y >= 0) // now the guard is moving right
							dir = Character::Direction::DOWN;
						else
							dir = Character::Direction::UP;
					}


					// switch its direction
					guardObj.SwitchDirection(dir, glfwGetTime());
				}
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

		// update guard's appearance
		registry.renderRequests.get(guard).used_texture = guardObj.GetTexId(glfwGetTime());
	}

	return true;
}

// button handlers for level selections
// void WorldSystem::level1ButtonHandler() {
// 	if (1 <= gameState->gameLevel.unlockedLevel) {
// 		gameState->gameLevel.currLevel = 1;
// 	}
// 	// TODO: display something different when the user can't access a certain level
// }




// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset Camera
	renderer->translationMatrix = { {-0.5f, 0.f, 0.f}, {0.f, 1.0f, 0.f}, {0.f, 0.f, 0.f} };

	// Reset the trap effect
	while (registry.trappables.entities.size() > 0)
		registry.remove_all_components_of(registry.trappables.entities.back());

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create new game state if we don't already have a game state
	GameState* gameState = NULL;
	if (registry.gameStates.size() == 0) {
		gameStateEntity = createGameState();
		gameState = &registry.gameStates.get(gameStateEntity);
		// Load Level From File
		// gameState->gameLevel.saveLevelToFile(); // TODO: delete this later
		gameState->gameLevel.loadlLevelFromFile();
		//gameState->gameLevel.saveLevelToFile();
	}

	gameState = &registry.gameStates.get(gameStateEntity);
	// TODO: depend on current gameState, we either display menu or the game of a specific level
	if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTION) {
		// if we are in level selection menu

		// Create Level Selection
		float paddingFactor = 10;
		createTextBox(renderer, { bg_X / 2, 4 * (bg_Y - 50) / paddingFactor }, TEXTURE_ASSET_ID::TUTORIAL_BUTTON, BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "show tutorial");
		createTextBox(renderer, { bg_X / 2, 5* (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(1), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level1");
		createTextBox(renderer, { bg_X / 2, 6 * (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(2), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level2");
		createTextBox(renderer, { bg_X / 2, 7 * (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(3), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level3");
		createTextBox(renderer, { bg_X / 2, 8 * (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(4), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level4");
		createTextBox(renderer, { bg_X / 2, 9 * (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(5), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level5");
		createTextBox(renderer, { bg_X / 2, 10 * (bg_Y - 50) / paddingFactor }, getTextureIDOfLevelButton(6), BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT, "level6");
	}
	else if (gameState->state == GameState::GAME_STATE::TUTORIAL_PAGE) {
		showTutorial();
	}
	else if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTED) {
		// user selected level, display game component of that level
		showLevelContent(gameState->gameLevel, gameState->gameLevel.currLevel);
		

		// TODO: select game level 2-6 and display correspouding content here
		
	}

	

	
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

TEXTURE_ASSET_ID WorldSystem::getTextureIDOfLevelButton(int level) {
	int unlocked_id = 59 + (level - 1) * 2 + 1; // skip other texture + skip prev levels + current level
	int locked_id = unlocked_id + 1;
	TEXTURE_ASSET_ID texture_id;
	GameState* gameState = &registry.gameStates.get(gameStateEntity);
	if (gameState->gameLevel.unlockedLevel >= level) {
		texture_id = static_cast<TEXTURE_ASSET_ID>(unlocked_id);
	}
	else {
		texture_id = static_cast<TEXTURE_ASSET_ID>(locked_id);
	}
	return texture_id;

}

// called in restart_game, display game content for game level 1
void WorldSystem::showLevelContent(GameLevel& level, int levelIndex) {
	std::ifstream in(level_map_path("level" + std::to_string(levelIndex) + ".txt"));
	std::string map_row;
	level.levelMap.clear();
	std::vector<std::vector<char>>& level_map = level.levelMap;


	while (std::getline(in, map_row)) {
		std::vector<char> charVector(map_row.begin(), map_row.end());
		level_map.push_back(charVector);
	}

	for (int row = 0; row < level_map.size(); row++) {
		for (int col = 0; col < level_map[row].size(); col++) {
			if (level_map[row][col] == 'W') {
				createWall(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'T') {
				createTrap(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'E') {
				exit = createExit(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'S') {
				player_student = createStudent(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'G') {
				if (levelIndex == 3) {
					guard = createGuard(renderer, { col * WALL_SIZE, row * WALL_SIZE }, { 0.f, 0.f });
				}
				else {
					guard = createGuard(renderer, { col * WALL_SIZE, row * WALL_SIZE }, { -100.f, 0.f });
				}
			}
			else if (level_map[row][col] == 'C') {
				createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 1);
			}
			else if (level_map[row][col] == 'L') {
				createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 3);
			}
			else if (level_map[row][col] == 'N') {
				createNPC(renderer,  { col * WALL_SIZE, row * WALL_SIZE });
			}
		}
	}

	registry.colors.insert(player_student, { 1, 0.8f, 0.8f });
}

// display tutorial image
void WorldSystem::showTutorial() {
	createTextBox(renderer, {bg_X / 2, 2*bg_Y / 3}, TEXTURE_ASSET_ID::TUTORIAL_CONTENT, TUTORIAL_BB_WIDTH, TUTORIAL_BB_HEIGHT, "none");
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions; // TODO: @Tim, is the reference here needed?
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// if the guard is in collisions with the wall
		if (registry.guards.has(entity) && registry.walls.has(entity_other))
		{
			//cout << "collision" << endl;
			auto & v = registry.motions.get(guard).velocity;
			auto & vGoal = registry.motions.get(guard).velocityGoal;
			//v = vec2(0);
			//vGoal = -vGoal;
		}

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the chicken sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, death_sound, 0);
					registry.motions.get(entity_other).velocity = { 0, 0 };
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

			// wall
			else if (registry.stopables.has(entity_other)) {
				if (!registry.stopeds.has(entity)) {
					registry.stopeds.emplace(entity);
				}

				vec2 velocity = registry.motions.get(entity).velocityGoal;
				vec2 position = registry.motions.get(entity).position;

				if (abs(velocity.x) > abs(velocity.y) || abs(registry.motions.get(entity).velocity.x) > abs(registry.motions.get(entity).velocity.y)) {
					if (velocity.x > 0 || registry.motions.get(entity).velocity.x >0) {
						left_to_right = 1;
						registry.motions.get(entity).velocityGoal = { 0, 0 };
						registry.motions.get(entity).velocity = { 0, 0 };
						registry.motions.get(entity).position = { position.x - 5.f, position.y };
					}
					else if (velocity.x < 0 || registry.motions.get(entity).velocity.x <0) {
						left_to_right = 0;
						registry.motions.get(entity).velocityGoal = { 0, 0 };
						registry.motions.get(entity).velocity = { 0, 0 };
						registry.motions.get(entity).position = { position.x + 5.f, position.y };
					}
					else {
						if (left_to_right) {
							registry.motions.get(entity).position = { position.x - 5.f, position.y };
						}
						else {
							registry.motions.get(entity).position = { position.x + 5.f, position.y };
						}
					}
				}
				else if (abs(velocity.x) <= abs(velocity.y) || abs(registry.motions.get(entity).velocity.x) <= abs(registry.motions.get(entity).velocity.y)) {
					if (velocity.y > 0 || registry.motions.get(entity).velocity.y>0) {
						bot_to_top = 0;
						registry.motions.get(entity).velocityGoal = { 0, 0 };
						registry.motions.get(entity).velocity = { 0, 0 };
						registry.motions.get(entity).position = { position.x, position.y - 5.f };
					}
					else if (velocity.y < 0 || registry.motions.get(entity).velocity.y<0) {
						bot_to_top = 1;
						registry.motions.get(entity).velocityGoal = { 0, 0 };
						registry.motions.get(entity).velocity = { 0, 0 };
						registry.motions.get(entity).position = { position.x, position.y + 5.f };
					}
					else {
						if (bot_to_top) {
							
							registry.motions.get(entity).position = { position.x, position.y + 5.f };
						}
						else {
							registry.motions.get(entity).position = { position.x, position.y - 5.f };
						}
						
					}
				}

				//registry.motions.get(entity).position = { position.x, position.y };
			}
			else if (registry.wins.has(entity_other)) {
				Mix_PlayChannel(-1, fire_alarm_sound, 2);
				if (!registry.wins.has(entity)) {
					registry.wins.emplace(entity);
				}
				//registry.clear_all_components();
				createTextBox(renderer, { bg_X / 2, bg_Y / 2 }, TEXTURE_ASSET_ID::WIN, WIN_BB_WIDTH, WIN_BB_HEIGHT, "unlock new level");
			}
			else if (registry.traps.has(entity_other)) {
				Mix_PlayChannel(-1, trap_sound, 1);
				registry.remove_all_components_of(entity_other);
				registry.trappables.emplace(entity_other);

				// Remove all the guards from the walkTimers component container since the player interact with a trap
				// TODO: may not need to remove all the guards, just some of them
				for (Entity entity : registry.walkTimers.entities) {
					registry.walkTimers.remove(entity);
					registry.motions.get(entity).velocity = { 0 , 0 };
				}
			}
			// check if the user collided with an NPC with conversation
			else if (registry.conversations.has(entity_other)) {
				Conversation& conversation = registry.conversations.get(entity_other);
				if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::CRIME_DETECTED) {
					std::cout << "already talked, crime detected" << std::endl;
				} else if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::NO_CRIME_DETECTED) {
					std::cout << "already talked, nocrime detected" << std::endl;
				} else if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::NO_CONVERSATION_YET) {
					// first time meeting student, change photo rendered
					RenderRequest& renderRequest = registry.renderRequests.get(conversation.textBox);
					conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::DURING_CONVERSATION);
					renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_DURING_CONVERSATION;
				}
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

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
		if (!isFullScreen) {
			glfwSetWindowMonitor(window, monitor, 0, 0, window_width_px, window_height_px, GLFW_DONT_CARE);
			isFullScreen = true;
		} else {
			glfwSetWindowMonitor(window, nullptr, mode->width / 4, mode->height / 4, window_width_px, window_height_px, GLFW_DONT_CARE);
			isFullScreen = false;
		}
	}

	GameState* gameState = &registry.gameStates.get(gameStateEntity);
	if (gameState->state == GameState::GAME_STATE::TUTORIAL_PAGE) {
		std::cout << "On key for tutorial page is pressed" << std::endl;
		if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
			// go back to menu
			std::cout << "M key pressed and released" << std::endl;
			gameState->state = GameState::GAME_STATE::LEVEL_SELECTION;
			restart_game();
		}
	}
	else if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTED) {
		Motion& motion = registry.motions.get(player_student);

		mat3 currPosition = renderer->translationMatrix;

		if (registry.wins.has(player_student)) {
			return;
		}

		// detect NPC conversation changes
		for (auto& npc: registry.conversations.entities) {
			std::cout << "conversation checking..." << std::endl;
			auto& conversation = registry.conversations.get(npc);
			if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::DURING_CONVERSATION) {
				std::cout << "during conversation" << std::endl;
				RenderRequest& renderRequest = registry.renderRequests.get(conversation.textBox);
				if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
					conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::NO_CRIME_DETECTED);
					renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_NO_CRIME_DETECTED;
					std::cout << "Z released" << std::endl;
				} else if  (key == GLFW_KEY_X && action == GLFW_RELEASE) {
					conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::CRIME_DETECTED);
					renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_CRIME_DETECTED;
					std::cout << "X released" << std::endl;
				}
			}
		}

		// get player instance's reference
		auto& player = registry.players.get(player_student);

		if (key == GLFW_KEY_W) {
			registry.stopeds.remove(player_student);
			if (action == GLFW_PRESS) {

				motion.velocityGoal = { 0,-PLAYER_SPEED };

				// refresh player's direction
				player.SwitchDirection(Player::Direction::UP, glfwGetTime());
			}
			else if (action == GLFW_RELEASE) {
				motion.velocityGoal = { 0,0 };
			}
		}
		if (key == GLFW_KEY_S) {
			registry.stopeds.remove(player_student);
			if (action == GLFW_PRESS) {
				motion.velocityGoal = { 0,PLAYER_SPEED };

				// refresh player's direction
				player.SwitchDirection(Player::Direction::DOWN, glfwGetTime());
			}
			else if (action == GLFW_RELEASE) {
				motion.velocityGoal = { 0,0 };
			}
		}
		if (key == GLFW_KEY_A) {
			registry.stopeds.remove(player_student);
			if (action == GLFW_PRESS) {
				motion.velocityGoal = { -PLAYER_SPEED,0 };

				// refresh player's direction
				player.SwitchDirection(Player::Direction::LEFT, glfwGetTime());
			}
			else if (action == GLFW_RELEASE) {
				motion.velocityGoal = { 0,0 };
			}
		}
		if (key == GLFW_KEY_D) {
			registry.stopeds.remove(player_student);
			if (action == GLFW_PRESS) {
				motion.velocityGoal = { PLAYER_SPEED,0 };

				// refresh player's direction
				player.SwitchDirection(Player::Direction::RIGHT, glfwGetTime());
			}
			else if (action == GLFW_RELEASE) {
				motion.velocityGoal = { 0,0 };
			}
		}

		// get the reference of the texture id that player is using
		auto& playerUsedTex = registry.renderRequests.get(player_student).used_texture;

		// update player's appearance
		playerUsedTex = player.GetTexId(glfwGetTime());

		/// .----------------------------------------
		if (key == GLFW_KEY_UP) {
			if (action == GLFW_PRESS && currPosition[1].y > -0.950) {
				renderer->translationMatrix[1].y = currPosition[1].y - 0.05;
			}
		}
		if (key == GLFW_KEY_DOWN) {
			if (action == GLFW_PRESS && currPosition[1].y < 0.950) {
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
	
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the chicken's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//(vec2)mouse_position; // dummy to avoid compiler warning

	cursorX = mouse_position[0];
	cursorY = mouse_position[1];
	//std::cout << cursorX << " " << cursorY << std::endl;
	GameState* gameState = &registry.gameStates.get(gameStateEntity);
	if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTION) {
	}
}

bool inRange(vec2 buttonPos, int buttonWidth, int buttonHeight) {
	float buttonLeft = buttonPos.x - (buttonWidth / 2);
	float buttonRight = buttonPos.x + (buttonWidth / 2);
	float buttonTop = buttonPos.y - (buttonHeight / 2);
	float buttonBot = buttonPos.y + (buttonHeight / 2);
	float normalizedCursorX = cursorX * 1.5; // TODO: this is related to background size
	float normalizedCursorY = cursorY + (550 - 160);
	bool xInRange = (buttonLeft <= normalizedCursorX) && (buttonRight >= normalizedCursorX);
	bool yInRange = (buttonTop <= normalizedCursorY) && (buttonBot >= normalizedCursorY);
	//std::cout << "buttonPos " << buttonPos[0] << " " << buttonPos[1] << std::endl;
	//std::cout << "buttonWidth " << buttonWidth << std::endl;
	//std::cout << "buttonHeight " << buttonHeight << std::endl;
	//std::cout << "normalizedCursorX " << normalizedCursorX << std::endl;
	//std::cout << "normalizedCursorY " << normalizedCursorY << std::endl;
	//std::cout << "buttonTop " << buttonTop << std::endl;
	//std::cout << "buttonBot " << buttonBot << std::endl;
	//std::cout << "xInRange " << xInRange << std::endl;
	//std::cout << "yInRange " << yInRange << std::endl;
	return xInRange && yInRange;
}


Clickable* findClickedButton() {
	// uses cursorX and cursorY to see if it is in range of any button
	//std::cout << "findClickedButton" << std::endl;
	for (auto &c : registry.clickables.components) {
		//std::cout << "----check button range ----" << std::endl;
		if (inRange(c.position, c.width, c.height)) {
			//std::cout << "Found Clicked Button" << std::endl;
			return &c; // TODO: does this work?
		}
	}
	return NULL;
}

// return -1 if level unchanged
// return the new level if success
int WorldSystem::changeLevel(std::string buttonAction) {
	int switchToLevel = -1;
	if (buttonAction == "level1") {
		switchToLevel = 1;
	}
	else if (buttonAction == "level2") {
		switchToLevel = 2;
	}
	else if (buttonAction == "level3") {
		switchToLevel = 3;
	}
	else if (buttonAction == "level4") {
		switchToLevel = 4;
	}
	else if (buttonAction == "level5") {
		switchToLevel = 5;
	}
	else if (buttonAction == "level6") {
		switchToLevel = 6;
	}

	if (switchToLevel != -1) {
		GameState* gameState = &registry.gameStates.get(gameStateEntity);
		if (switchToLevel <= gameState->gameLevel.unlockedLevel) {
			gameState->state = GameState::GAME_STATE::LEVEL_SELECTED;
			gameState->gameLevel.currLevel = switchToLevel;
		}
		else {
			//std::cout << "Tried to access locked level, failed" << std::endl;
			switchToLevel = -1;
		}
	}
	
	return switchToLevel;

	// change level to switchToLevel

}


void WorldSystem::mouse_button_callback(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		//std::cout << "Left Button Released" << std::endl;

		// clickable component that we clicked on
		GameState* gameState = &registry.gameStates.get(gameStateEntity);
		Clickable* clickedClickable = findClickedButton();
		if (clickedClickable != NULL) {
			// get button action
			std::string buttonAction = clickedClickable->buttonAction;

			if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTION) {
				// if we are on level selection page
				//std::cout << "Selecting Levls" << std::endl;

				// check if user clicked on the level selection button
				int new_level = changeLevel(buttonAction);
				if (new_level != -1) {
					//std::cout << "new_level is: " << new_level << std::endl;
					restart_game();
					return; // we changed the level
				}

				// check if user clicked tutorial button
				if (buttonAction == "show tutorial") {
					gameState->state = GameState::GAME_STATE::TUTORIAL_PAGE;
					restart_game();
				}

			}

			
			
			
		}

		// go back to menu if winned the game
		if (gameState->state == GameState::GAME_STATE::LEVEL_SELECTED && registry.wins.has(player_student)) {
			// currently in an game
			//std::cout << "Playing in level finished a level and go back to menu" << std::endl;
			// unlock the next level and go back to home page

			// check if we are not at maximum level + we are playing the last unlocked level
			if (gameState->gameLevel.unlockedLevel < MAX_LEVEL && gameState->gameLevel.unlockedLevel == gameState->gameLevel.currLevel) {
			
				gameState->gameLevel.unlockedLevel += 1;
				gameState->gameLevel.saveLevelToFile();
			}
			gameState->state = GameState::GAME_STATE::LEVEL_SELECTION;
			restart_game();
		}
	}
}
float WorldSystem::approach(float goal_v, float cur_v, float dt)
{
	float diff = goal_v - cur_v;

	if (diff > dt) {
		return cur_v + dt;
	}
	if (diff < -dt) {
		return cur_v - dt;
	}

	return goal_v;
}

