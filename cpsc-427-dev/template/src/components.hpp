#pragma once
#include "common.hpp"
#include "game_state.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "conversation_state.hpp"

enum class TEXTURE_ASSET_ID;

enum class Direction { UP, LEFT, DOWN, RIGHT };

struct Movie
{
public:
	Movie(std::vector<TEXTURE_ASSET_ID> textures, double frameInterval = 0.1); // frameInterval: units s(seconds)

	void SetTextures(std::vector<TEXTURE_ASSET_ID> textures);

	virtual TEXTURE_ASSET_ID GetTexId(double nowTime);
protected:
	int curTexIndex;
	std::vector<TEXTURE_ASSET_ID> textures;
	double frameInterval; // while running, every duration of "frameInterval" switch an apperance
	double lastSwitchTime;
};

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

// tool
struct Tool
{
	enum class ToolType{SANDGLASS, REMOTE_CONTROL, HAMMER, BEE};

	Tool(ToolType type);

	TEXTURE_ASSET_ID GetTexId(double nowTime);

	vec2 GetUIPosition()const;

	vec2 GetUISize()const;

	char GetCommandChar()const;

private:
	ToolType type;
	Movie movie;
};


struct Guard
{

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

struct Camera
{

};

struct Light
{

};

struct Trappable
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10, 10 };
	vec2 velocityGoal = { 0.f, 0.f };
	
	Motion() {}

	void to_json(json& j, const Motion& motion) {
        j["position.x"] = motion.position.x;
		j["position.y"] = motion.position.y;
		j["angle"] = motion.angle;
		j["velocity.x"] = motion.velocity.x;
		j["velocity.y"] = motion.velocity.y;
		j["scale.x"] = motion.scale.x;
		j["scale.y"] = motion.scale.y;
		j["velocityGoal.x"] = motion.velocityGoal.x;
		j["velocityGoal.y"] = motion.velocityGoal.y;

    }

	void from_json(const json& j, Motion& motion) {
        j.at("position.x").get_to(motion.position.x);
        j.at("position.y").get_to(motion.position.y);
		j.at("angle").get_to(motion.angle);
		j.at("velocity.x").get_to(motion.velocity.x);
		j.at("velocity.y").get_to(motion.velocity.y);
		j.at("scale.x").get_to(motion.scale.x);
		j.at("scale.y").get_to(motion.scale.y);
		j.at("velocityGoal.x").get_to(motion.velocityGoal.x);
		j.at("velocityGoal.y").get_to(motion.velocityGoal.y);
    }
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
struct TurnTimer {
	float counter_ms = 0;

	TurnTimer(float turnTime) :turnTime(turnTime),counter_ms(turnTime) {}

	bool UpdateAndCheckIsTimeout(float elapsed_ms);

private:
	float turnTime;
};

//struct RotateTimer {
//	float counter_ms = 3000;
//};

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

struct Clickable
{
	Clickable(vec2 p, float w, float h, std::string ba) {
		position = p;
		width = w;
		height = h;
		buttonAction = ba;
	}

	vec2 position;
	float width;
	float height;
	std::string buttonAction;
	// void (*handler) ();
};
struct Trap
{

};
struct Conversation
{
	Conversation(Entity tb) {
		conversationState = ConversationState();
		textBox = tb;
	}
	ConversationState conversationState;
	Entity textBox;
};

// this struct is used for marking which entity is a ui element
struct UI
{

};

struct Background {

};

// this struct is used for exploding
struct Exploded
{
	float life; // decrease every loop
	float initLife;
	vec2 initSize;
	Exploded(float life,vec2 initSize) :life(life),initLife(life),initSize(initSize) {}
};

/*

below is an example of a wind's influence range:
    
          |----------|    -
 pos -->  |-->-->--> |    |<-width
          |----------|    -

          |<-length->|

	 dir = Direction::RIGHT
*/
struct Wind
{
	vec2 pos;
	float width; // the width of wind
	float length; // the length of wind
	Direction dir;
	int particleCount;
	Wind(vec2 pos, float width, float length, Direction dir) :pos(pos), width(width), length(length), dir(dir), particleCount(0) {}

	bool InRange(vec2 objPos) const;

	// make a motion be influence by this wind
	void Influence(Motion &motion) const;

	// get a direction by a character that "<>v^" means left/right/down/up
	static Direction GetWindDirByChar(char c);
	static char GetWindDirChar(Direction dir);
};


struct WindParticle
{
private:
	float t0; // born time
	vec2 pos0; // born position
	float v; // velocity at flowing direction
	float A; // amplitude of sin curve
	float T; // period of sin curve
	float parentLength;
public:
	Entity windEntity;
	Direction dir;
	WindParticle(const Wind &parentWind, const Entity &windEntity, float t0);

	// x = x0 + v * t
	// y = y0 + A sin(2*pi/T * t)
	vec2 GetPos(float t) const;
	bool IsAlive(float t) const;
};

struct Bee
{
private:
	float usedTime;
	vec2 diffPosWithTarget;
	vec2 bornPos;
	float goingTime;
	float stayTime;
	float leaveTime;
	float A;// amplitude of sin curve
	float T; // period of sin curve
public:
	Entity targetEntity; // the target purchased by bee
	enum class State{GOING,STAY,LEAVE};
	//State state;

	Bee(const Entity &targetEntity, vec2 diffPosWithTarget, vec2 bornPos);

	State GetState() const;
	void ModifyMotion(float dt, Motion &beeMotion, const Motion &targetMotion);
	bool IsAlive() const;
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
	CAMERA = WIN + 1,
	LIGHT = CAMERA + 1,
	PLAYER_DOWN_0 = LIGHT + 1,
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
	TRAP = GUARD_RIGHT_8 + 1,
	LEVEL1 = TRAP + 1,
	LEVEL1_LOCKED = LEVEL1 + 1,
	LEVEL2 = LEVEL1_LOCKED + 1,
	LEVEL2_LOCKED = LEVEL2 + 1,
	LEVEL3 = LEVEL2_LOCKED + 1,
	LEVEL3_LOCKED = LEVEL3 + 1,
	LEVEL4 = LEVEL3_LOCKED + 1,
	LEVEL4_LOCKED = LEVEL4 + 1,
	LEVEL5 = LEVEL4_LOCKED + 1,
	LEVEL5_LOCKED = LEVEL5 + 1,
	LEVEL6 = LEVEL5_LOCKED + 1,
	LEVEL6_LOCKED = LEVEL6 + 1,
	TUTORIAL_BUTTON = LEVEL6_LOCKED + 1,
	TUTORIAL_CONTENT = TUTORIAL_BUTTON + 1,
	NPC_STUDENT = TUTORIAL_CONTENT + 1,
	NPC_NO_CONVERSATION = NPC_STUDENT + 1,
	NPC_DURING_CONVERSATION = NPC_NO_CONVERSATION + 1,
	NPC_CRIME_DETECTED = NPC_DURING_CONVERSATION + 1,
	NPC_NO_CRIME_DETECTED = NPC_CRIME_DETECTED + 1,
	TITLE = NPC_NO_CRIME_DETECTED + 1,
	PRESS_ANY = TITLE + 1,
	SANDGLASS = PRESS_ANY + 1,
	REMOTE_CONTROL = SANDGLASS + 1,
	HAMMER = REMOTE_CONTROL + 1,
	BEE = HAMMER + 1,
	COVER0 = BEE + 1,
	COVER1 = COVER0 + 1,
	COVER2 = COVER1 + 1,
	COVER3 = COVER2 + 1,
	TOOL_GRID = COVER3 + 1,
	SELECTION_BG= TOOL_GRID + 1,
	FLOOR_BG = SELECTION_BG + 1,
	WIND_PARTICLE = FLOOR_BG + 1,
	TEXTURE_COUNT = WIND_PARTICLE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	EGG=0,
	WIND = EGG+1,
	TEXTURED = WIND+1,
	UI= TEXTURED +1,
	EFFECT_COUNT = UI + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	BUG = 0,
	DEBUG_LINE = BUG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	SPRITE = SCREEN_TRIANGLE + 1,
	GEOMETRY_COUNT = SPRITE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	bool showOnMinimap;

	void changeTexture(TEXTURE_ASSET_ID newTexture) {
		used_texture = newTexture;
	}

	RenderRequest(TEXTURE_ASSET_ID used_texture, EFFECT_ASSET_ID used_effect, GEOMETRY_BUFFER_ID used_geometry, bool showOnMinimap) :
		used_texture(used_texture),
		used_effect(used_effect),
		used_geometry(used_geometry),
		showOnMinimap(showOnMinimap)
	{}
};

