#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <string>
#include <random>

using namespace std;

std::default_random_engine eng;

Entity createStudent(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.velocityGoal = { 0.f, 0.f };
	// motion.scale = mesh.original_size * 300.f;
	// motion.scale.y *= -1; // point front to the right
	motion.scale = vec2({ STUDENT_BB_WIDTH, STUDENT_BB_HEIGHT });

	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_UP_0, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		true });

	return entity;
}

Entity createWall(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &wall = registry.walls.emplace(entity);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -WALL_BB_WIDTH, WALL_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.stopables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WALL, // TODo
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		true });

	return entity;
}

Entity createExit(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &exit = registry.exits.emplace(entity);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ EXIT_BB_WIDTH, EXIT_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.wins.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::EXIT, // TODo
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createGuard(RenderSystem *renderer, vec2 position, vec2 v)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize walk timer
	registry.turnTimers.emplace(entity, GUARD_TURN_TIME);

	// Create and (empty) Guard component to be able to refer to all guards
	registry.guards.emplace(entity);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	// motion.angle = 0.f;
	motion.velocityGoal = v;
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ GUARD_BB_WIDTH, GUARD_BB_HEIGHT });

	// Create and (empty) Eagle component to be able to refer to all eagles
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::EAGLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}


Entity createCamera(RenderSystem *renderer, vec2 position, uint16_t direction)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);

	// Setting initial values, scale is negative to make it face the opposite way
	if (direction == 0) {
		motion.scale = { -CAMERA_BB_WIDTH, CAMERA_BB_HEIGHT };
		motion.position = { position.x - 5.f, position.y - 5.f };
	}
	else if (direction == 1) {
		motion.scale = { CAMERA_BB_WIDTH, CAMERA_BB_HEIGHT };
		motion.position = { position.x + 5.f, position.y - 5.f };
	}
	else if (direction == 2) {
		motion.scale = { -CAMERA_BB_WIDTH, CAMERA_BB_HEIGHT };
		motion.position = { position.x - 5.f, position.y + 5.f };
	}
	else {
		motion.scale = { CAMERA_BB_WIDTH, CAMERA_BB_HEIGHT };
		motion.position = { position.x + 5.f, position.y + 5.f };
	}

	// Create and (empty) Eagle component to be able to refer to all eagles
	registry.cameras.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CAMERA,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createLight(RenderSystem *renderer, vec2 position, uint16_t direction)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize rotate timer
	registry.turnTimers.emplace(entity, LIGHT_TURN_TIME);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	if (direction == 0) {
		motion.angle = 0.5f;
		motion.velocity = { 0.5f, 0 };
		motion.position = { position.x - 10.f, position.y - 10.f };
		motion.scale = { LIGHT_BB_WIDTH, LIGHT_BB_HEIGHT };
	}
	else if (direction == 1) {
		motion.angle = -0.5f;
		motion.velocity = { 0.5f, 0 };
		motion.position = { position.x + 10.f, position.y - 10.f };
		motion.scale = { -LIGHT_BB_WIDTH, LIGHT_BB_HEIGHT };
	}
	else if (direction == 2) {
		motion.angle = 0.5f;
		motion.velocity = { -0.5f, 0 };
		motion.position = { position.x - 10.f, position.y + 10.f };
		motion.scale = { LIGHT_BB_WIDTH, LIGHT_BB_HEIGHT };
	}
	else {
		motion.angle = 0.5f;
		motion.velocity = { -0.5f, 0 };
		motion.position = { position.x + 10.f, position.y + 10.f };
		motion.scale = { -LIGHT_BB_WIDTH, LIGHT_BB_HEIGHT };
	}


	// Create and (empty) Eagle component to be able to refer to all eagles
	registry.lights.emplace(entity);
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::LIGHT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createUIBox(RenderSystem *renderer, vec2 position, vec2 size, enum TEXTURE_ASSET_ID textureAssetId, std::string buttonAction) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;
	motion.scale = size;

	if (!buttonAction.empty())
	{
		// Initialize Clickable component with position, size and hanlder
		auto &clickable = registry.clickables.emplace(entity, position, size.x, size.y, buttonAction);
	}


	registry.uis.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ textureAssetId, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createWind(RenderSystem *renderer, vec2 position, float width, float length, Direction dir)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;

	registry.winds.emplace(entity, position, width, length, dir);

	// no rendering but let it born particles at render function

	//registry.renderRequests.insert(
	//	entity,
	//	{ textureAssetId, // TEXTURE_ASSET_ID
	//	 EFFECT_ASSET_ID::UI,
	//	 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createWindParticle(RenderSystem *renderer, const Wind &wind, const Entity &windEntity)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	WindParticle &windParticle = registry.windParticles.emplace(entity, wind, windEntity, glfwGetTime());

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = windParticle.GetPos(glfwGetTime());
	motion.scale = vec2(WALL_SIZE * 0.25f);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WIND_PARTICLE, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createBee(RenderSystem *renderer, const Entity &targetEntity, vec2 beeBornPos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	uniform_real_distribution<float> uni(-GUARD_BB_WIDTH, GUARD_BB_WIDTH);
	vec2 diffPosWithTarget(uni(eng), uni(eng));

	Bee &bee = registry.bees.emplace(entity, targetEntity, diffPosWithTarget, beeBornPos);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = beeBornPos;
	motion.scale = vec2(WALL_SIZE * 0.5f);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BEE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createTrapUI(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	//auto& trap = registry.traps.emplace(entity);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	// motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ TRAP_BB_WIDTH, TRAP_BB_HEIGHT }); // TODO

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TRAP,
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false });

	return entity;
}

Entity createMultiply(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { MULTI_BB_SIZE, MULTI_BB_SIZE };

	registry.uis.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MULTI,
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false });

	return entity;
}

Entity createDigit(RenderSystem* renderer, vec2 position, int digit) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { DIGIT_BB_WIDTH, DIGIT_BB_HEIGHT };

	registry.uis.emplace(entity);

	enum TEXTURE_ASSET_ID textureAssetId;
	switch (digit) {
		case 0: textureAssetId = TEXTURE_ASSET_ID::DIGIT0; break;
		case 1: textureAssetId = TEXTURE_ASSET_ID::DIGIT1; break;
		case 2: textureAssetId = TEXTURE_ASSET_ID::DIGIT2; break;
		case 3: textureAssetId = TEXTURE_ASSET_ID::DIGIT3; break;
		case 4: textureAssetId = TEXTURE_ASSET_ID::DIGIT4; break;
		case 5: textureAssetId = TEXTURE_ASSET_ID::DIGIT5; break;
		case 6: textureAssetId = TEXTURE_ASSET_ID::DIGIT6; break;
		case 7: textureAssetId = TEXTURE_ASSET_ID::DIGIT7; break;
		case 8: textureAssetId = TEXTURE_ASSET_ID::DIGIT8; break;
		case 9: textureAssetId = TEXTURE_ASSET_ID::DIGIT9; break;
		default: textureAssetId = TEXTURE_ASSET_ID::DIGIT0; break;
	}

	registry.renderRequests.insert(
		entity,
		{ textureAssetId, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false });

	return entity;
}

Entity createRecord(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { RECORD_BB_WIDTH, RECORD_BB_HEIGHT };

	registry.uis.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::RECORD,
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false });

	return entity;
}

Entity createHighest(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { HIGHEST_BB_WIDTH, HIGHEST_BB_HEIGHT };

	registry.uis.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HIGHEST,
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false });

	return entity;
}

Entity createBackground(RenderSystem *renderer, vec2 position, vec2 size, enum TEXTURE_ASSET_ID textureAssetId) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;
	motion.scale = size;

	registry.background.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ textureAssetId, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createTextBox(RenderSystem *renderer, vec2 position, enum TEXTURE_ASSET_ID textureAssetId, float width, float height, std::string buttonAction) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	// motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;

	// Initialize Clickable component with position, size and hanlder
	auto &clickable = registry.clickables.emplace(entity, position, width, height, buttonAction);

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ width, height }); // TODO

	registry.renderRequests.insert(
		entity,
		{ textureAssetId, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createTrap(RenderSystem *renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &trap = registry.traps.emplace(entity);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	// motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ TRAP_BB_WIDTH, TRAP_BB_HEIGHT }); // TODO

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TRAP,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE,
		false});

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

// Reload game state from file, create game state
Entity createGameState() {
	Entity entity = Entity();

	GameState &gameState = registry.gameStates.emplace(entity);
	return entity;
}

// Create NPC that talk to the users
Entity createNPC(RenderSystem *renderer, vec2 position)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	//motion.scale = mesh.original_size * 2.f;
	//motion.scale.x *= -1;
	motion.scale = vec2({ -NPC_BB_WIDTH, NPC_BB_HEIGHT });

	// Intilize conversation component of this NPC
	Entity textBox = createTextBox(renderer, { position.x - 10, position.y - 100 }, TEXTURE_ASSET_ID::NPC_NO_CONVERSATION, CONVERSATION_BB_WIDTH, CONVERSATION_BB_HEIGHT, "none");
	registry.conversations.emplace(entity, textBox);
	registry.uis.emplace(textBox);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::NPC_STUDENT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		true});

	return entity;

}

Entity createMovie(RenderSystem *renderer, vec2 pos, vec2 size, std::vector<TEXTURE_ASSET_ID> textures, double frameInterval)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	//Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0,0 };
	motion.position = pos;
	motion.scale = size;

	// Initialize Clickable component with position, size and hanlder
	auto &clickable = registry.clickables.emplace(entity, pos, size.x, size.y, "");


	registry.movies.emplace(entity, textures, frameInterval);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BUG, // TEXTURE_ASSET_ID
		 EFFECT_ASSET_ID::UI,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

Entity createTool(RenderSystem *renderer, vec2 position, Tool::ToolType type)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ NPC_BB_WIDTH, NPC_BB_HEIGHT });

	Tool &tool = registry.tools.emplace(entity, type);

	registry.eatables.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ tool.GetTexId(0),
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		false});

	return entity;
}

void createExplodeds(RenderSystem *renderer, int count, vec2 position, vec2 size, TEXTURE_ASSET_ID textureAssetId, float life)
{
	std::uniform_real_distribution<double> uni(0, 1);

	for (int i = 0; i < count; ++i)
	{
		auto entity = Entity();

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);


		//Initialize the motion
		float v0 = 50 + uni(eng) * 350; // initialized abs of velocity
		float angle = uni(eng) * 360; // emit direction angle
		vec2 initSize = (float)uni(eng) * size;

		auto &motion = registry.motions.emplace(entity);
		motion.angle = angle;
		motion.velocity = { v0 * cos(angle),v0 * sin(angle) };
		motion.position = position;
		motion.scale = initSize;

		registry.explodeds.emplace(entity, life, initSize);

		registry.renderRequests.insert(
			entity,
			{ textureAssetId,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			true});
	}
	return;
}
