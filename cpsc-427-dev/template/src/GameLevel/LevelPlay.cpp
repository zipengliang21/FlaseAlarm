#include "LevelPlay.h"

#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"
#include "render_system.hpp"

#include <string>

using namespace std;

// Game configuration
const size_t MAX_BUG = 5;
const float PLAYER_SPEED = 200;


int bot_to_top = 0;
int left_to_right = 0;

LevelPlay::LevelPlay(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window) :GameLevel(renderer, manager, window)
, next_bug_spawn(0.f)
{
	// Reset the game speed
	current_speed = 1.f;

	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	assert(chicken_dead_sound);

	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
	assert(chicken_eat_sound);

	death_sound = Mix_LoadWAV(audio_path("death.wav").c_str());
	assert(death_sound);

	fire_alarm_sound = Mix_LoadWAV(audio_path("fire_alarm.wav").c_str());
	assert(fire_alarm_sound);

	trap_sound = Mix_LoadWAV(audio_path("trap.wav").c_str());
	assert(trap_sound);

	explode_sound = Mix_LoadWAV(audio_path("explode.wav").c_str());
	assert(explode_sound);

	startLevel_sound = Mix_LoadWAV(audio_path("start.wav").c_str());
	assert(startLevel_sound);
}

LevelPlay::~LevelPlay()
{
	// Destroy music components
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	if (death_sound != nullptr)
		Mix_FreeChunk(death_sound);
	if (fire_alarm_sound != nullptr)
		Mix_FreeChunk(fire_alarm_sound);
	if (explode_sound != nullptr)
		Mix_FreeChunk(explode_sound);
	if (startLevel_sound != nullptr)
		Mix_FreeChunk(startLevel_sound);
	Mix_CloseAudio();
}

void LevelPlay::step(float elapsed_ms)
{
	ProcessKeyPress();

	UpdateWindParticle();

	UpdateBee(elapsed_ms / 1000.0f);

	// traversal the count down events, if time out, run it.
	for (auto it = countdownEvents.begin(); it != countdownEvents.end();)
	{
		auto &ev = *it;
		if (glfwGetTime() > ev.runTime) // should call
		{
			// call
			ev.cb();

			// earse this event
			it = countdownEvents.erase(it);
		}
		else
		{
			it++;
		}
	}

	ai.step(elapsed_ms);

	// update player velocity

	Motion &player_motion = registry.motions.get(player);
	Motion &guard_motion = registry.motions.get(guard);

	player_motion.velocity.x = approach(player_motion.velocityGoal.x, player_motion.velocity.x, elapsed_ms);
	player_motion.velocity.y = approach(player_motion.velocityGoal.y, player_motion.velocity.y, elapsed_ms);

	guard_motion.velocity.x = approach(guard_motion.velocityGoal.x, guard_motion.velocity.x, elapsed_ms);
	guard_motion.velocity.y = approach(guard_motion.velocityGoal.y, guard_motion.velocity.y, elapsed_ms);


	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
		Motion &motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Spawning new bug
	next_bug_spawn -= elapsed_ms * current_speed;
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
		DeathTimer &counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			// go back to menu when restarting the game
			//Restart();
			manager->GoLevelSelection();
			return;
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity entity : registry.winTimers.entities) {
		WinTimer &counter = registry.winTimers.get(entity);
		counter.counter_ms -= elapsed_ms;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.winTimers.remove(entity);
			screen.greener_screen_factor = 0;

			// go back to menu when restarting the game
			Restart();

			manager->GoLevelSelection();
			return;
		}
	}

	// turn screen to grren
	screen.greener_screen_factor = 1 - min_counter_ms / 3000;

	// get the guard instance
	auto &guardObj = registry.deadlys.get(guard);
	auto &guardMotion = registry.motions.get(guard);
	//printf("%f,%f\n", mo.position.x, mo.position.y);

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter
	for (Entity entity : registry.turnTimers.entities) {
		TurnTimer &counter = registry.turnTimers.get(entity);
		Motion &motion = registry.motions.get(entity);

		if (counter.UpdateAndCheckIsTimeout(elapsed_ms))
		{
			if (entity == guard)
			{
				motion.velocityGoal = { -1 * motion.velocityGoal[0] , motion.velocityGoal[1] }; // make the guard turn over

			}

			if (registry.lights.has(entity))
			{
				motion.velocity.x = -motion.velocity.x; // make lights turn over
			}
		}
	}

	{
		Character::Direction dir;
		if (abs(guardMotion.velocity.x) >= abs(guardMotion.velocity.y)) {
			if (guardMotion.velocity.x >= 0) // now the guard is moving right
				dir = Character::Direction::RIGHT;
			else
				dir = Character::Direction::LEFT;
		}
		else {
			if (guardMotion.velocity.y >= 0) // now the guard is moving down
				dir = Character::Direction::DOWN;
			else
				dir = Character::Direction::UP;
		}

		// switch its direction
		guardObj.SwitchDirection(dir, glfwGetTime());

		// update guard's appearance
		registry.renderRequests.get(guard).used_texture = guardObj.GetTexId(glfwGetTime());
	}

	// the translation matrix is:
	//         [ 1 0 diffX ]
	//         [ 0 1 diffY ]
	//         [ 1 0    1  ]
	// but the opengl's matrix is column-first. so the matrix's index is as below.
	renderer->viewMatrix[2][0] = -player_motion.position.x + window_width_px / 2.0;
	renderer->viewMatrix[2][1] = -player_motion.position.y + window_height_px / 2.0;

	// set renderer's mask properties
	renderer->playerPos = player_motion.position;

	// -------- end of step() -----------
}

void LevelPlay::handle_collisions()
{
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
			auto &v = registry.motions.get(guard).velocity;
			auto &vGoal = registry.motions.get(guard).velocityGoal;
			//v = vec2(0);
			//vGoal = -vGoal;
		}

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			if (if_collisions_player_with_deadly(entity_other))
				continue;

			if (if_collisions_player_with_eatable(entity_other))
				continue;

			if (if_collisions_player_with_stopable(entity_other))
				continue;

			if (if_collisions_player_with_wins(entity_other))
				continue;

			if (if_collisions_player_with_traps(entity_other))
				continue;

			if (if_collisions_player_with_conversations(entity_other))
				continue;
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void LevelPlay::OnKey(int key, int, int action, int mod)
{
	Motion &motion = registry.motions.get(player);

	//mat3 currPosition = renderer->translationMatrix;

	if (registry.wins.has(player)) {
		return;
	}

	// detect NPC conversation changes
	for (auto &npc : registry.conversations.entities) {
		//std::cout << "conversation checking..." << std::endl;
		auto &conversation = registry.conversations.get(npc);
		if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::DURING_CONVERSATION) {
			std::cout << "during conversation" << std::endl;
			RenderRequest &renderRequest = registry.renderRequests.get(conversation.textBox);
			if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
				conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::NO_CRIME_DETECTED);
				renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_NO_CRIME_DETECTED;
				std::cout << "Z released" << std::endl;
			}
			else if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
				conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::CRIME_DETECTED);
				renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_CRIME_DETECTED;
				std::cout << "X released" << std::endl;
			}
		}
	}

	// get player instance's reference
	auto &playerInst = registry.players.get(player);

	if (key == GLFW_KEY_W && !registry.stopeds.has(player)) {
		//registry.stopeds.remove(player);
		if (action == GLFW_PRESS) {

			motion.velocityGoal = { 0,-PLAYER_SPEED };

			// refresh player's direction
			playerInst.SwitchDirection(Player::Direction::UP, glfwGetTime());
		}
		else if (action == GLFW_RELEASE && motion.velocityGoal.x == 0 && motion.velocityGoal.y == -PLAYER_SPEED) {
			motion.velocityGoal = { 0,0 };
		}
	}
	else if (key == GLFW_KEY_S && !registry.stopeds.has(player)) {
		//registry.stopeds.remove(player);
		if (action == GLFW_PRESS) {
			motion.velocityGoal = { 0,PLAYER_SPEED };

			// refresh player's direction
			playerInst.SwitchDirection(Player::Direction::DOWN, glfwGetTime());
		}
		else if (action == GLFW_RELEASE && motion.velocityGoal.x == 0 && motion.velocityGoal.y == PLAYER_SPEED) {
			motion.velocityGoal = { 0,0 };
		}
	}
	else if (key == GLFW_KEY_A && !registry.stopeds.has(player)) {
		//registry.stopeds.remove(player);
		if (action == GLFW_PRESS) {
			motion.velocityGoal = { -PLAYER_SPEED,0 };

			// refresh player's direction
			playerInst.SwitchDirection(Player::Direction::LEFT, glfwGetTime());
		}
		else if (action == GLFW_RELEASE && motion.velocityGoal.x == -PLAYER_SPEED && motion.velocityGoal.y == 0) {
			motion.velocityGoal = { 0,0 };
		}
	}
	else if (key == GLFW_KEY_D && !registry.stopeds.has(player)) {
		//registry.stopeds.remove(player);
		if (action == GLFW_PRESS) {
			motion.velocityGoal = { PLAYER_SPEED,0 };

			// refresh player's direction
			playerInst.SwitchDirection(Player::Direction::RIGHT, glfwGetTime());
		}
		else if (action == GLFW_RELEASE && motion.velocityGoal.x == PLAYER_SPEED && motion.velocityGoal.y == 0) {
			motion.velocityGoal = { 0,0 };
		}
	}

	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		// go back to menu if someone clicked M
		manager->GoLevelSelection();
		return;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		if (registry.stopeds.has(player)) {
			registry.stopeds.remove(player);
		}
	}

	// get the reference of the texture id that player is using
	auto &playerUsedTex = registry.renderRequests.get(player).used_texture;

	// update player's appearance
	playerUsedTex = playerInst.GetTexId(glfwGetTime());

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

void LevelPlay::OnMouseMove(double x, double y)
{
	for (auto &e : hoverHammer)
	{
		auto &motion = registry.motions.get(e);
		motion.position = { x,y };
	}
}

void LevelPlay::OnMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		// clickable component that we clicked on

		// get cursor
		double cursorX, cursorY;
		glfwGetCursorPos(window, &cursorX, &cursorY);

		// go back to menu if winned the game
		if (registry.wins.has(player)) {
			// currently in an game
			// unlock the next level and go back to home page

			GameState *gameState = &registry.gameStates.get(manager->gameStateEntity);
			gameState->WinAtLevel(gameState->GetCurrentLevelIndex());
			gameState->SetCurrentLevelIndex(-1);

			manager->GoLevelSelection();
			return;
		}

		// just for hammer function
		do
		{
			if (hoverHammer.empty()) // hammer is not hovering
				break;

			int row, col;
			if (GetClickedRowCol({ cursorX,cursorY }, row, col) == false) // not click in map range
				break;

			auto it = walls.find({ row,col });
			if (it == walls.end()) // not click a wall
				break;

			cout << "clicked wall" << endl;

			// store the walls that should be broken
			vector<Entity> shouldBreakWall;

			auto &gameMap = registry.gameStates.get(manager->gameStateEntity).GetCurrentMap();
			const int range = 3; // break range

			// get all the neighbour wall in range
			for (int i = glm::max(0, row - range); i < glm::min((int)gameMap.size(), row + range); ++i)
				for (int j = glm::max(0, col - range); j < glm::min((int)gameMap[i].size(), col + range); ++j)
				{
					if (gameMap[i][j] == 'W')
					{
						auto it2 = walls.find({ i,j });
						if (it2 != walls.end())
						{
							shouldBreakWall.push_back(it2->second);
							walls.erase(it2);
						}
					}
				}

			// remove walls
			for (auto &wall : shouldBreakWall)
			{
				registry.remove_all_components_of(wall);
			}

			// add explode effects
			createExplodeds(renderer, 40, vec2(col * WALL_SIZE, row * WALL_SIZE), vec2(WALL_SIZE), TEXTURE_ASSET_ID::WALL, 1500);

			// remove this hammer
			Entity hammer = *hoverHammer.begin();
			registry.remove_all_components_of(hammer);
			hoverHammer.erase(hoverHammer.begin());

			// play sound TODO: replace this sound
			Mix_PlayChannel(-1, explode_sound, 0);
			return;

		} while (0);

		Entity entity;
		if (findClickedEntity(cursorX, cursorY, entity) == false)
			return;

		Clickable &clickable = registry.clickables.get(entity);
		if (clickable.buttonAction == to_string(SANDGLASS_CHAR)) // clicked sandglass
		{
			if_clicked_sandglass_button(entity);
			return;
		}

		if (clickable.buttonAction == to_string(REMOTE_CONTROL_CHAR)) // clicked remote control
		{
			if_clicked_remote_control_button(entity);
			return;
		}

		if (clickable.buttonAction == to_string(HAMMER_CHAR)) // clicked hammer
		{
			hoverHammer.insert(entity);
			return;
		}

		if (clickable.buttonAction == to_string(BEE_CHAR)) // clicked bee
		{
			if_clicked_bee_button(entity);
			return;
		}

		assert(0 && "uncompleted button action");
	}
}

void LevelPlay::ProcessKeyPress()
{
	if (registry.stopeds.has(player))
		return;

	// get player instance's reference
	auto &playerInst = registry.players.get(player);

	// if ASDW key pressing, make player refresh its appearance
	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) ||
		(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) ||
		(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) ||
		(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
	{
		// get the reference of the texture id that player is using
		auto &playerUsedTex = registry.renderRequests.get(player).used_texture;

		// update player's appearance
		playerUsedTex = playerInst.GetTexId(glfwGetTime());
	}

	return; // turn out the code below

	Motion &motion = registry.motions.get(player);

	motion.velocityGoal = { 0,0 };
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		motion.velocityGoal.y = -PLAYER_SPEED;
		//registry.players.get(player).SwitchDirection(Character::Direction::UP, glfwGetTime());
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		motion.velocityGoal.y = PLAYER_SPEED;
		//registry.players.get(player).SwitchDirection(Character::Direction::DOWN, glfwGetTime());
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		motion.velocityGoal.x = -PLAYER_SPEED;
		//registry.players.get(player).SwitchDirection(Character::Direction::LEFT, glfwGetTime());
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		motion.velocityGoal.x = PLAYER_SPEED;
		//registry.players.get(player).SwitchDirection(Character::Direction::RIGHT, glfwGetTime());
	}
}

bool LevelPlay::if_collisions_player_with_deadly(Entity other)
{
	// Checking Player - Deadly collisions
	if (registry.deadlys.has(other)) {
		// initiate death unless already dying
		if (!registry.deathTimers.has(player)) {
			Mix_PlayChannel(-1, death_sound, 0);
			// Scream, reset timer, and make the chicken sink
			registry.deathTimers.emplace(player);
			registry.motions.get(other).velocity = { 0, 0 };
			// set saved history to invliad if dead
			GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
			gameState.jsonObject["stateIsValid"] = false;
			// write json to file
			std::ofstream o(GameStatus_JSON_FILE_PATH);
			o << gameState.jsonObject << std::endl;
		}
		return true;
	}
	return false;
}

bool LevelPlay::if_collisions_player_with_eatable(Entity other)
{
	// now, only tools can be ate.

	// Checking Player - Eatable collisions
	if (registry.eatables.has(other)) {
		if (!registry.deathTimers.has(player))  // not dead
		{
			Tool &tool = registry.tools.get(other);
			createUIBox(renderer, tool.GetUIPosition(), tool.GetUISize(), tool.GetTexId(glfwGetTime()), to_string(tool.GetCommandChar()));

			// chew, count points, and set the LightUp timer
			registry.remove_all_components_of(other);
			Mix_PlayChannel(-1, chicken_eat_sound, 0);

			// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the chicken entity by modifying the ECS registry
		}
		return true;
	}
	return false;
}

bool LevelPlay::if_collisions_player_with_stopable(Entity other)
{
	// wall
	if (registry.stopables.has(other)) {
		if (!registry.stopeds.has(player)) {
			registry.stopeds.emplace(player);
		}
		else {
			return true;
		}
		vec2 velocity = registry.motions.get(player).velocityGoal;

		vec2 position = registry.motions.get(player).position;
		vec2 position_other = registry.motions.get(other).position;

		vec2 diff = position_other - position;
		if (abs(diff.x) > abs(diff.y)) {
			if (diff.x > 0) {
				registry.motions.get(player).velocityGoal = { 0, 0 };
				registry.motions.get(player).velocity = { 0, 0 };
				registry.motions.get(player).position = { position.x - 15.f, position.y };
			}
			else if (diff.x < 0) {
				registry.motions.get(player).velocityGoal = { 0, 0 };
				registry.motions.get(player).velocity = { 0, 0 };
				registry.motions.get(player).position = { position.x + 15.f, position.y };
			}
		}
		else {
			if (diff.y > 0) {
				registry.motions.get(player).velocityGoal = { 0, 0 };
				registry.motions.get(player).velocity = { 0, 0 };
				registry.motions.get(player).position = { position.x, position.y - 15.f };
			}
			else if (diff.y < 0) {
				registry.motions.get(player).velocityGoal = { 0, 0 };
				registry.motions.get(player).velocity = { 0, 0 };
				registry.motions.get(player).position = { position.x, position.y + 15.f };
			}
		}
		//registry.motions.get(entity).position = { position.x, position.y };
		return true;
	}
	return false;
}

bool LevelPlay::if_collisions_player_with_wins(Entity other)
{
	if (registry.wins.has(other)) {
		// win

		Mix_PlayChannel(-1, fire_alarm_sound, 2);
		if (!registry.wins.has(player)) {
			registry.wins.emplace(player);
		}
		//registry.clear_all_components();

		// show at center of window
		createUIBox(renderer, { window_width_px / 2.0,window_height_px / 2.0 }, { WIN_BB_WIDTH, WIN_BB_HEIGHT }, TEXTURE_ASSET_ID::WIN, "unlock new level");
		// set saved history to invliad if win
		GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
		gameState.jsonObject["stateIsValid"] = false;
		// write json to file
		std::ofstream o(GameStatus_JSON_FILE_PATH);
		o << gameState.jsonObject << std::endl;
		return true;
	}
	return false;
}

bool LevelPlay::if_collisions_player_with_traps(Entity other)
{
	if (registry.traps.has(other)) {
		// trap

		Mix_PlayChannel(-1, trap_sound, 1);
		registry.remove_all_components_of(other);
		registry.trappables.emplace(other);

		// Remove all the guards from the walkTimers component container since the player interact with a trap
		// TODO: may not need to remove all the guards, just some of them
		for (Entity entity : registry.turnTimers.entities) {
			if (registry.guards.has(entity))
			{
				registry.turnTimers.remove(entity); // guard will not be turning around
				registry.motions.get(entity).velocity = { 0 , 0 };
			}
		}
		return true;
	}
	return false;
}

bool LevelPlay::if_collisions_player_with_conversations(Entity other)
{
	// check if the user collided with an NPC with conversation
	if (registry.conversations.has(other)) {
		Conversation &conversation = registry.conversations.get(other);
		if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::CRIME_DETECTED) {
			std::cout << "already talked, crime detected" << std::endl;
		}
		else if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::NO_CRIME_DETECTED) {
			std::cout << "already talked, nocrime detected" << std::endl;
		}
		else if (conversation.conversationState.getState() == ConversationState::CONVERSATION_STATE::NO_CONVERSATION_YET) {
			// first time meeting student, change photo rendered
			RenderRequest &renderRequest = registry.renderRequests.get(conversation.textBox);
			conversation.conversationState.setState(ConversationState::CONVERSATION_STATE::DURING_CONVERSATION);
			renderRequest.used_texture = TEXTURE_ASSET_ID::NPC_DURING_CONVERSATION;
		}
		return true;
	}
	return false;
}

void LevelPlay::if_clicked_sandglass_button(Entity entity)
{
	const float stopTime = 5.0f;

	// for guard
	auto &guardMotion = registry.motions.get(guard);

	// insert it into the events, this callback function will be called at step() when timeout
	countdownEvents.emplace_back(glfwGetTime() + stopTime, [=]()
		{
			registry.motions.get(guard) = guardMotion;
		});

	// make guard stop
	guardMotion.velocity = { 0,0 };
	guardMotion.velocityGoal = { 0,0 };

	// event for restoring all the lights
	for (Entity light : registry.lights.entities)
	{
		auto &motion = registry.motions.get(light);

		// insert it into the events, this callback function will be called at step() when timeout
		countdownEvents.emplace_back(glfwGetTime() + stopTime, [=]()
			{
				if (registry.motions.has(light)) // if player uses remote control, light is deleted
					registry.motions.get(light) = motion;
			});

		// make all lights stop
		motion.velocity = { 0,0 };
		motion.velocityGoal = { 0,0 };
	}

	// event for restoring turnTimers
	for (Entity &counter : registry.turnTimers.entities)
	{
		auto origin_counter_ms = registry.turnTimers.get(counter).counter_ms;
		// insert it into the events, this callback function will be called at step() when timeout
		countdownEvents.emplace_back(glfwGetTime() + stopTime, [=]()
			{
				if (registry.turnTimers.has(counter)) // if player touched trap, counter is deleted
					registry.turnTimers.get(counter).counter_ms = origin_counter_ms;
			});
	}

	// stop ai and register its restore event
	ai.SetEnable(false);
	countdownEvents.emplace_back(glfwGetTime() + stopTime, [=]()
		{
			ai.SetEnable(true);
		});

	// this ui is not shown anymore
	registry.clickables.remove(entity);
	registry.renderRequests.remove(entity);
}

void LevelPlay::if_clicked_remote_control_button(Entity entity)
{
	// remove all the lights and relative objects
	for (Entity light : registry.lights.entities)
	{
		registry.remove_all_components_of(light);
	}

	// this callback funtion is used for reloading all the light from map
	auto RestoreLight = [&]()
	{
		GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
		auto &level_map = gameState.GetCurrentMap();
		// recreate entity
		for (int row = 0; row < level_map.size(); row++) {
			for (int col = 0; col < level_map[row].size(); col++)
			{

				if (level_map[row][col] == 'H') {
					createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 0);
				}
				else if (level_map[row][col] == 'J') {
					createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 1);
				}
				else if (level_map[row][col] == 'K') {
					createLight(renderer, { col * WALL_SIZE + 40, row * WALL_SIZE - 7 }, 2);
				}
				else if (level_map[row][col] == 'L') {
					createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 3);
				}
			}
		}

		for (int row = 0; row < level_map.size(); row++) {
			for (int col = 0; col < level_map[row].size(); col++)
			{
				if (level_map[row][col] == 'Z') {
					createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 0);
				}
					else if (level_map[row][col] == 'X') {
					createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 1);
				}
					else if (level_map[row][col] == 'C') {
					createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 2);
				}
					else if (level_map[row][col] == 'V') {
					createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 3);
				}
			}
		}
	};

	// register a callback event for restoring all the lights
	countdownEvents.emplace_back(glfwGetTime() + 5.0f, RestoreLight);

	// this ui is not shown anymore
	registry.clickables.remove(entity);
	registry.renderRequests.remove(entity);
}

void LevelPlay::if_clicked_bee_button(Entity entity)
{
	GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
	auto &levelMap = gameState.GetCurrentMap();

	vec2 beeBornPos(levelMap[0].size() * WALL_SIZE, levelMap.size() * WALL_SIZE);

	// add bees
	for (int i = 0; i < 10; ++i)
	{
		createBee(renderer, guard, beeBornPos);
	}


	// for guard
	float tStop = glfwGetTime() + BEE_GOING_TIME;
	float tRestore = glfwGetTime() + BEE_GOING_TIME + BEE_STAY_TIME;
	auto StopGuard = [=]()
	{
		// the ref of guard's motion
		Motion &guardMotion = registry.motions.get(guard);

		// insert it into the events, this callback function will be called at step() when timeout
		this->countdownEvents.emplace_back(tRestore, [=]()
			{
				registry.motions.get(guard) = guardMotion;
			});

		// make guard stop
		guardMotion.velocity = { 0,0 };
		guardMotion.velocityGoal = { 0,0 };

		// event for restoring turnTimers
		for (Entity &counter : registry.turnTimers.entities)
		{
			float origin_counter_ms = registry.turnTimers.get(counter).counter_ms;
			// insert it into the events, this callback function will be called at step() when timeout
			countdownEvents.emplace_back(tRestore, [=]()
				{
					if (registry.turnTimers.has(counter)) // if player touched trap, counter is deleted
						registry.turnTimers.get(counter).counter_ms = origin_counter_ms;
				});
		}

		// stop ai and register its restore event
		ai.SetEnable(false);
		countdownEvents.emplace_back(tRestore, [=]()
			{
				ai.SetEnable(true);
			});
	};

	// do register
	countdownEvents.emplace_back(tStop, StopGuard);


	// this ui is not shown anymore
	registry.clickables.remove(entity);
	registry.renderRequests.remove(entity);
}

bool LevelPlay::GetClickedRowCol(vec2 cursor, int &row, int &col)
{
	GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
	auto &level_map = gameState.GetCurrentMap();

	auto &playerMotion = registry.motions.get(player);
	vec2 mapPos = cursor - vec2(window_width_px, window_height_px) * 0.5f + playerMotion.position;
	//cout <<"mapPos="<< mapPos.x << "," << mapPos.y << endl;

	row = mapPos.y / WALL_SIZE;
	col = mapPos.x / WALL_SIZE;
	//cout << "row,col=" <<row << "," << col << endl;

	if (row >= 0 && row < level_map.size() &&
		col >= 0 && col < level_map[row].size())
	{
		//cout << level_map[row][col] << endl;
		return true;
	}

	return false;
}

void LevelPlay::UpdateWindParticle()
{
	// calc all the wind particles 's life and erase dead instance
	for (auto &e : registry.windParticles.entities)
	{
		WindParticle &inst = registry.windParticles.get(e);
		if (inst.IsAlive(glfwGetTime()) == false)
		{
			//cout << "-" << Wind::GetWindDirChar(inst.dir) << endl;

			//
			auto &wind = inst.windEntity;
			if (registry.winds.has(wind))
			{
				registry.winds.get(wind).particleCount--;
			}

			registry.remove_all_components_of(e);
			continue;
		}

		// if it's alive, update its position
		auto &motion = registry.motions.get(e);
		motion.position = inst.GetPos(glfwGetTime());
	}

	// add wind particles
	{
		for (auto &wind : registry.winds.entities)
		{
			auto &inst = registry.winds.get(wind);
			//cout << Wind::GetWindDirChar(inst.dir) << ":" << inst.particleCount << endl;
			if (inst.particleCount < WIND_PARTICLE_LIMIT)
			{
				createWindParticle(renderer, inst, wind);
				inst.particleCount++;
				//cout << Wind::GetWindDirChar(inst.dir) << endl;
			}
		}
	}
}

void LevelPlay::UpdateBee(float dt)
{
	auto &targetMotion = registry.motions.get(guard);
	for (auto &e : registry.bees.entities)
	{
		auto &motion = registry.motions.get(e);
		auto &inst = registry.bees.get(e);

		inst.ModifyMotion(dt, motion, targetMotion);

		if (inst.IsAlive() == false)
		{
			registry.remove_all_components_of(e);
		}
	}
}

void LevelPlay::Restart()
{
	// Reset the game speed
	current_speed = 1.f;

	if (registry.gameStates.size() == 0) {
		//gameState->gameLevel.saveLevelToFile();
	}

	hoverHammer.clear();
	walls.clear();
	countdownEvents.clear();

	GameState &gameState = registry.gameStates.get(manager->gameStateEntity);
	auto &level_map = gameState.GetCurrentMap();

	float w = window_width_px;
	float h = window_height_px;

	//add background 
	const vec2 &mapSize = gameState.GetMapPixelSize();
	vec2 bgCenter = mapSize / 2.0f;
	vec2 bgSize(mapSize.x + window_width_px, mapSize.y + window_height_px);
	createBackground(renderer, bgCenter, bgSize, TEXTURE_ASSET_ID::FLOOR_BG);

	// recreate entity
	for (int row = 0; row < level_map.size(); row++) {
		for (int col = 0; col < level_map[row].size(); col++)
		{
			if (level_map[row][col] == 'W')
			{
				Entity wall = createWall(renderer, { col * WALL_SIZE, row * WALL_SIZE });
				walls[{row, col}] = wall;
			}
			else if (level_map[row][col] == 'T') {
				createTrap(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'E') {
				exit = createExit(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == 'S') {
				player = createStudent(renderer, { col * WALL_SIZE, row * WALL_SIZE });
				// see if we recovers players' position
				if (gameState.savedState == 1) {
					Motion& playerMotion = registry.motions.get(player);
					playerMotion.from_json(gameState.jsonObject, playerMotion);
				}
			}
			else if (level_map[row][col] == 'G') {
				if (gameState.GetCurrentLevelIndex() != 1 && gameState.GetCurrentLevelIndex() != 2) {
					guard = createGuard(renderer, { col * WALL_SIZE, row * WALL_SIZE }, { 0.f, 0.f });
				}
				else {
					guard = createGuard(renderer, { col * WALL_SIZE, row * WALL_SIZE }, { -50.f, 0.f });
				}
			}
			else if (level_map[row][col] == 'Z') {
				createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 0);
			}
			else if (level_map[row][col] == 'X') {
				createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 1);
			}
			else if (level_map[row][col] == 'C') {
				createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 2);
			}
			else if (level_map[row][col] == 'V') {
				createCamera(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 3);
			}
			else if (level_map[row][col] == 'H') {
				createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 0);
			}
			else if (level_map[row][col] == 'J') {
				createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 1);
			}
			else if (level_map[row][col] == 'K') {
				createLight(renderer, { col * WALL_SIZE + 40, row * WALL_SIZE - 7 }, 2);
			}
			else if (level_map[row][col] == 'L') {
				createLight(renderer, { col * WALL_SIZE, row * WALL_SIZE }, 3);
			}
			else if (level_map[row][col] == 'N') {
				createNPC(renderer, { col * WALL_SIZE, row * WALL_SIZE });
			}
			else if (level_map[row][col] == SANDGLASS_CHAR)
			{
				createTool(renderer, { col * WALL_SIZE, row * WALL_SIZE }, Tool::ToolType::SANDGLASS);
			}
			else if (level_map[row][col] == REMOTE_CONTROL_CHAR)
			{
				createTool(renderer, { col * WALL_SIZE, row * WALL_SIZE }, Tool::ToolType::REMOTE_CONTROL);
			}
			else if (level_map[row][col] == HAMMER_CHAR)
			{
				createTool(renderer, { col * WALL_SIZE, row * WALL_SIZE }, Tool::ToolType::HAMMER);
			}
			else if (level_map[row][col] == BEE_CHAR)
			{
				createTool(renderer, { col * WALL_SIZE, row * WALL_SIZE }, Tool::ToolType::BEE);
			}
			else if (level_map[row][col] == '<' | level_map[row][col] == '>' | level_map[row][col] == '^' | level_map[row][col] == 'v')
			{
				createWind(renderer, { col * WALL_SIZE, row * WALL_SIZE }, WIND_WIDTH_SIZE, WIND_LENGTH_SIZE, Wind::GetWindDirByChar(level_map[row][col]));
			}
			else
			{
				assert("undefined map element");
			}
		}
	}

	// set saved state to 0, delete previous state
	gameState.savedState = 0;

	// play start level music
	Mix_PlayChannel(-1, startLevel_sound, 0);

	registry.colors.insert(player, { 1, 0.8f, 0.8f });

	// add tool grid
	createUIBox(renderer, { window_width_px * TOOL1_UI_X_POS_COEF,window_height_px * 0.1 }, { window_width_px * 0.1, window_width_px * 0.1 }, TEXTURE_ASSET_ID::TOOL_GRID, "");
	createUIBox(renderer, { window_width_px * TOOL2_UI_X_POS_COEF,window_height_px * 0.1 }, { window_width_px * 0.1, window_width_px * 0.1 }, TEXTURE_ASSET_ID::TOOL_GRID, "");
	createUIBox(renderer, { window_width_px * TOOL3_UI_X_POS_COEF,window_height_px * 0.1 }, { window_width_px * 0.1, window_width_px * 0.1 }, TEXTURE_ASSET_ID::TOOL_GRID, "");
	createUIBox(renderer, { window_width_px * TOOL4_UI_X_POS_COEF,window_height_px * 0.1 }, { window_width_px * 0.1, window_width_px * 0.1 }, TEXTURE_ASSET_ID::TOOL_GRID, "");

	renderer->useMask = true;
}

float approach(float goal_v, float cur_v, float dt)
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
