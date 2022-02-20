#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

enum class TEXTURE_ASSET_ID;

// the generic character that can show 4-direction's appearance
struct Character
{
	enum class Direction { UP, LEFT, DOWN, RIGHT };
	Character() :lastSwitchTime(0) {}
	Character &operator=(const Character &other) {
		curDir = other.curDir; curTexId = other.curTexId; lastSwitchTime = other.lastSwitchTime;
		return *this;
	}

	// return the texture id and refresh status. every frame should call it once.
	virtual TEXTURE_ASSET_ID GetTexId(double nowTime)=0;// pure-virtual function

	// switch direction
	virtual void SwitchDirection(Direction dir, double nowTime)=0;
protected:
	Direction curDir;
	TEXTURE_ASSET_ID curTexId;
	const double switchFrame = 0.1; // while running, every duration of "switchFrame" switch an apperance
	double lastSwitchTime;
};

// Player component
struct Player:public Character
{
	// return the texture id and refresh status. every frame should call it once.
	virtual TEXTURE_ASSET_ID GetTexId(double nowTime);

	// switch direction
	void SwitchDirection(Direction dir, double nowTime);
};

// Eagles have a hard shell
struct Deadly:public Character
{
	// set the default direction
	Deadly() {
		curDir = Character::Direction::LEFT;
	}

	// return the texture id and refresh status. every frame should call it once.
	virtual TEXTURE_ASSET_ID GetTexId(double nowTime);

	// switch direction
	void SwitchDirection(Direction dir, double nowTime);
};

// Bug and Chicken have a soft shell
struct Eatable
{

};

struct Stopable
{

};

struct Stoped
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10, 10 };
	vec2 velocityGoal = { 0.f, 0.f };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	float greener_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

// Timer that controls how long before guard turns around
struct WalkTimer {
	float counter_ms = 12000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Wall
{
	//vec2 position;
	
};

struct Exit
{

};

struct Win
{

};

struct WinTimer
{
	float counter_ms = 3000;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BUG = 0,
	EAGLE = BUG + 1,
	WALL = EAGLE + 1,
	EXIT = WALL + 1,
	WIN = EXIT + 1,
	PLAYER_DOWN_0 = WIN + 1,
	PLAYER_DOWN_1 = PLAYER_DOWN_0 + 1,
	PLAYER_DOWN_2 = PLAYER_DOWN_1 + 1,
	PLAYER_DOWN_3 = PLAYER_DOWN_2 + 1,
	PLAYER_UP_0 = PLAYER_DOWN_3 + 1,
	PLAYER_UP_1 = PLAYER_UP_0 + 1,
	PLAYER_UP_2 = PLAYER_UP_1 + 1,
	PLAYER_UP_3 = PLAYER_UP_2 + 1,
	PLAYER_LEFT_0 = PLAYER_UP_3 + 1,
	PLAYER_LEFT_1 = PLAYER_LEFT_0 + 1,
	PLAYER_LEFT_2 = PLAYER_LEFT_1 + 1,
	PLAYER_LEFT_3 = PLAYER_LEFT_2 + 1,
	PLAYER_RIGHT_0 = PLAYER_LEFT_3 + 1,
	PLAYER_RIGHT_1 = PLAYER_RIGHT_0 + 1,
	PLAYER_RIGHT_2 = PLAYER_RIGHT_1 + 1,
	PLAYER_RIGHT_3 = PLAYER_RIGHT_2 + 1,
	GUARD_UP_0 = PLAYER_RIGHT_3 + 1,
	GUARD_UP_1 = GUARD_UP_0 + 1,
	GUARD_UP_2 = GUARD_UP_1 + 1,
	GUARD_UP_3 = GUARD_UP_2 + 1,
	GUARD_UP_4 = GUARD_UP_3 + 1,
	GUARD_UP_5 = GUARD_UP_4 + 1,
	GUARD_UP_6 = GUARD_UP_5 + 1,
	GUARD_UP_7 = GUARD_UP_6 + 1,
	GUARD_UP_8 = GUARD_UP_7 + 1,
	GUARD_LEFT_0 = GUARD_UP_8 + 1,
	GUARD_LEFT_1 = GUARD_LEFT_0 + 1,
	GUARD_LEFT_2 = GUARD_LEFT_1 + 1,
	GUARD_LEFT_3 = GUARD_LEFT_2 + 1,
	GUARD_LEFT_4 = GUARD_LEFT_3 + 1,
	GUARD_LEFT_5 = GUARD_LEFT_4 + 1,
	GUARD_LEFT_6 = GUARD_LEFT_5 + 1,
	GUARD_LEFT_7 = GUARD_LEFT_6 + 1,
	GUARD_LEFT_8 = GUARD_LEFT_7 + 1,
	GUARD_DOWN_0 = GUARD_LEFT_8 + 1,
	GUARD_DOWN_1 = GUARD_DOWN_0 + 1,
	GUARD_DOWN_2 = GUARD_DOWN_1 + 1,
	GUARD_DOWN_3 = GUARD_DOWN_2 + 1,
	GUARD_DOWN_4 = GUARD_DOWN_3 + 1,
	GUARD_DOWN_5 = GUARD_DOWN_4 + 1,
	GUARD_DOWN_6 = GUARD_DOWN_5 + 1,
	GUARD_DOWN_7 = GUARD_DOWN_6 + 1,
	GUARD_DOWN_8 = GUARD_DOWN_7 + 1,
	GUARD_RIGHT_0 = GUARD_DOWN_8 + 1,
	GUARD_RIGHT_1 = GUARD_RIGHT_0 + 1,
	GUARD_RIGHT_2 = GUARD_RIGHT_1 + 1,
	GUARD_RIGHT_3 = GUARD_RIGHT_2 + 1,
	GUARD_RIGHT_4 = GUARD_RIGHT_3 + 1,
	GUARD_RIGHT_5 = GUARD_RIGHT_4 + 1,
	GUARD_RIGHT_6 = GUARD_RIGHT_5 + 1,
	GUARD_RIGHT_7 = GUARD_RIGHT_6 + 1,
	GUARD_RIGHT_8 = GUARD_RIGHT_7 + 1,
	TEXTURE_COUNT = GUARD_RIGHT_8 + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	WIND = TEXTURED + 1,
	EFFECT_COUNT = WIND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

