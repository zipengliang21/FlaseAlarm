#pragma once

#include "common.hpp"

class LevelManager;
class RenderSystem;

/*
* the common game level interface
* 
* This class's implement(subclass object) should be contained by a manager,
* the manager call the interface function for different behaviors.
*/
class GameLevel
{
public:
	GameLevel(RenderSystem *renderer,LevelManager *manager, GLFWwindow *window) :renderer(renderer),manager(manager),window(window) {}

	virtual ~GameLevel() {}

	// Steps the game ahead by ms milliseconds
	virtual void step(float elapsed_ms) {}

	// Check for collisions
	virtual void handle_collisions();

	// Reset the world state to its initial state
	// must call before clearing ECS 
	virtual void Restart() =0;

	// Input functions
	virtual void OnKey(int key, int, int action, int mod) {}

	virtual void OnMouseMove(double x, double y) {}

	virtual void OnMouseButton(int button, int action, int mods) {}

protected:
	RenderSystem *renderer;
	LevelManager *manager; 
	GLFWwindow *window;
};

bool inRange(vec2 buttonPos, int buttonWidth, int buttonHeight, double cursorX, double cursorY);

class Clickable;
Clickable *findClickedButton(double cursorX, double cursorY);

bool findClickedEntity(double cursorX, double cursorY, Entity &result);