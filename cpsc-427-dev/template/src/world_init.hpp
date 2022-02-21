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
const float CAMERA_BB_WEIGHT = 0.4f * 161.f;
const float CAMERA_BB_HEIGHT = 0.4f * 140.f;
const float LIGHT_BB_WEIGHT = 0.4f * 278.f;
const float LIGHT_BB_HEIGHT = 0.4f * 252.f;
const float TRAP_BB_WIDTH = 0.1f * 504.f;
const float TRAP_BB_HEIGHT = 0.15f * 444.f;

// the player
Entity createStudent(RenderSystem* renderer, vec2 pos);
// the guard
Entity createGuard(RenderSystem* renderer, vec2 position);
// the wall
Entity createWall(RenderSystem* renderer, vec2 position);
// the exit
Entity createExit(RenderSystem* renderer, vec2 position);
// the camera
Entity createCamera(RenderSystem* renderer, vec2 position);
// the light
Entity createLight(RenderSystem* renderer, vec2 position);
// the textbox
Entity createTextBox(RenderSystem* renderer, vec2 position);
// the trap
Entity createTrap(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);


