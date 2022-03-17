#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "conversation_state.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float STUDENT_BB_WIDTH = 0.3f * 165.f;
const float STUDENT_BB_HEIGHT = 0.3f * 165.f;
const float GUARD_BB_WIDTH = 0.2f * 512.f;
const float GUARD_BB_HEIGHT = 0.2f * 512.f;
const float NPC_BB_WIDTH = 0.3f * 165.f;
const float NPC_BB_HEIGHT = 0.3f * 165.f;
const float WALL_BB_WIDTH = 0.15f * 202.f;
const float WALL_BB_HEIGHT = 0.15f * 202.f;
const float EXIT_BB_WIDTH = 0.1f * 900.f;
const float EXIT_BB_HEIGHT = 0.1f * 1000.f;
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

// the player
Entity createStudent(RenderSystem* renderer, vec2 pos);
// the guard
Entity createGuard(RenderSystem* renderer, vec2 position, vec2 v);
// the wall
Entity createWall(RenderSystem* renderer, vec2 position);
// the exit
Entity createExit(RenderSystem* renderer, vec2 position);
// the textbox,  or button, display an image asset as a box
Entity createTextBox(RenderSystem* renderer, vec2 position, enum TEXTURE_ASSET_ID textureAssetId, float width, float height, std::string buttonAction);
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


