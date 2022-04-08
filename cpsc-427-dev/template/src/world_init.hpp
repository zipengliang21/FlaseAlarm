#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "conversation_state.hpp"

#include <random>

// These are ahrd coded to the dimensions of the entity texture
const float STUDENT_BB_WIDTH = 0.3f * 165.f;
const float STUDENT_BB_HEIGHT = 0.3f * 165.f;
const float GUARD_BB_WIDTH = 0.2f * 512.f;
const float GUARD_BB_HEIGHT = 0.2f * 512.f;
const float NPC_BB_WIDTH = 0.3f * 165.f;
const float NPC_BB_HEIGHT = 0.3f * 165.f;
const float WALL_BB_WIDTH = 0.15f * 202.f;
const float WALL_BB_HEIGHT = 0.15f * 202.f;
const float EXIT_BB_WIDTH = 0.1f * 474.f;
const float EXIT_BB_HEIGHT = 0.1f * 527.f;
const float WIN_BB_WIDTH = 0.4f * 512.f;
const float WIN_BB_HEIGHT = 0.4f * 512.f;
const float BUTTON_BB_WIDTH = 0.4f * 512.f;
const float BUTTON_BB_HEIGHT = 0.15f * 512.f;
const float CONVERSATION_BB_WIDTH = 1.1f * 512.f;
const float CONVERSATION_BB_HEIGHT = 0.13f * 512.f;
const float TUTORIAL_BB_WIDTH = 2.67f * 512.f;
const float TUTORIAL_BB_HEIGHT = 1.5f * 512.f;
const float CAMERA_BB_WIDTH = 0.4f * 161.f;
const float CAMERA_BB_HEIGHT = 0.4f * 94.f;
const float LIGHT_BB_WIDTH = 0.4f * 310.f;
const float LIGHT_BB_HEIGHT = 0.4f * 252.f;
const float TRAP_BB_WIDTH = 0.1f * 504.f;
const float TRAP_BB_HEIGHT = 0.15f * 444.f;
const float WALL_SIZE = 20.2f;

// tool
const float TOOL_UI_SIZE = 80.0f;
const float TOOL_UI_SIZE_COEF = 0.065;
const float TOOL1_UI_X_POS_COEF = 0.735;
const float TOOL2_UI_X_POS_COEF = TOOL1_UI_X_POS_COEF + TOOL_UI_SIZE_COEF;
const float TOOL3_UI_X_POS_COEF = TOOL2_UI_X_POS_COEF + TOOL_UI_SIZE_COEF;
const float TOOL4_UI_X_POS_COEF = TOOL3_UI_X_POS_COEF + TOOL_UI_SIZE_COEF;

const char SANDGLASS_CHAR = '1';
const char REMOTE_CONTROL_CHAR = '2';
const char HAMMER_CHAR = '3';
const char BEE_CHAR = '4';

// bee
const float BEE_GOING_TIME = 2.0f;
const float BEE_STAY_TIME = 5.0f;
const float BEE_LEAVE_TIME = 2.0f;

// wind
const float WIND_WIDTH_SIZE = WALL_SIZE * 3.0f;
const float WIND_LENGTH_SIZE = WALL_SIZE * 5.0f;
const int WIND_PARTICLE_LIMIT = 20;

// minimap
const float MINIMAP_WIDTH_COEF = 0.2f;
const float MINIMAP_POS_X = 20.0f;
const float MINIMAP_POS_Y = 20.0f;

const float GUARD_TURN_TIME = 12000; // units of ms
const float LIGHT_TURN_TIME = 3000; // units of ms

// point
const float MULTI_BB_SIZE = 0.15f * 315.f;
const float DIGIT_BB_WIDTH = 0.45f * 111.f;
const float DIGIT_BB_HEIGHT = 0.45f * 151.f;
const float RECORD_BB_WIDTH = 245.f;
const float RECORD_BB_HEIGHT = 27.f;
const float HIGHEST_BB_WIDTH = 1.08f * 283.f;
const float HIGHEST_BB_HEIGHT = 1.08f * 25.f;

extern std::default_random_engine eng;

// the player
Entity createStudent(RenderSystem* renderer, vec2 pos);
// the guard
Entity createGuard(RenderSystem* renderer, vec2 position, vec2 v);
// the wall
Entity createWall(RenderSystem* renderer, vec2 position);
// the exit
Entity createExit(RenderSystem* renderer, vec2 position);

// a ui box, means it will show on the fixed position at screen, whatever play is at any position and where the view is
// if buttonAction=="", no clickable will be created.
Entity createUIBox(RenderSystem *renderer, vec2 position, vec2 size, enum TEXTURE_ASSET_ID textureAssetId, std::string buttonAction);

// the textbox,  or button, display an image asset as a box
Entity createTextBox(RenderSystem *renderer, vec2 position, enum TEXTURE_ASSET_ID textureAssetId, float width, float height, std::string buttonAction);

// trap counter
Entity createTrapUI(RenderSystem* renderer, vec2 position);
Entity createMultiply(RenderSystem* renderer, vec2 position);
Entity createDigit(RenderSystem* renderer, vec2 position, int digit);

// the records
Entity createRecord(RenderSystem* renderer, vec2 position);
Entity createHighest(RenderSystem* renderer, vec2 position);

// the camera
Entity createCamera(RenderSystem* renderer, vec2 position, uint16_t direction);
// the light
Entity createLight(RenderSystem* renderer, vec2 position, uint16_t direction);
// the textbox
Entity createTextBox(RenderSystem* renderer, vec2 position);
// the trap
Entity createTrap(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// create game state
Entity createGameState();

// create NPC
Entity createNPC(RenderSystem* renderer, vec2 position);

// create movie
Entity createMovie(RenderSystem *renderer, vec2 pos, vec2 size, std::vector<TEXTURE_ASSET_ID> textures, double frameInterval);

Entity createTool(RenderSystem *renderer, vec2 position, Tool::ToolType type);

void createExplodeds(RenderSystem *renderer,int count, vec2 position, vec2 size, enum TEXTURE_ASSET_ID textureAssetId, float life);

Entity createWind(RenderSystem *renderer, vec2 position, float width, float length, Direction dir);
Entity createWindParticle(RenderSystem *renderer, const Wind &wind, const Entity &windEntity);

Entity createBee(RenderSystem *renderer, const Entity &targetEntity, vec2 beeBornPos);

// create background
Entity createBackground(RenderSystem* renderer, vec2 position, vec2 size, enum TEXTURE_ASSET_ID textureAssetId);