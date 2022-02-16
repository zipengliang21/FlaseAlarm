#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float STUDENT_BB_WIDTH = 0.3f * 165.f;
const float STUDENT_BB_HEIGHT = 0.3f * 165.f;
const float GUARD_BB_WIDTH = 0.2f * 512.f;
const float GUARD_BB_HEIGHT = 0.2f * 512.f;
const float WALL_BB_WIDTH = 0.1f * 202.f;
const float WALL_BB_HEIGHT = 0.1f * 202.f;
const float EXIT_BB_WIDTH = 0.1f * 900.f;
const float EXIT_BB_HEIGHT = 0.1f * 1000.f;
const float WIN_BB_WIDTH = 0.4f * 512.f;
const float WIN_BB_HEIGHT = 0.4f * 512.f;
const float BUTTON_BB_WIDTH = 0.4f * 512.f;
const float BUTTON_BB_HEIGHT = 0.15f * 512.f;

// the player
Entity createStudent(RenderSystem* renderer, vec2 pos);
// the guard
Entity createGuard(RenderSystem* renderer, vec2 position);
// the wall
Entity createWall(RenderSystem* renderer, vec2 position);
// the exit
Entity createExit(RenderSystem* renderer, vec2 position);
// the textbox,  or button, display an image asset as a box
Entity createTextBox(RenderSystem* renderer, vec2 position, enum TEXTURE_ASSET_ID textureAssetId, float width, float height);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);


