#pragma once
#include "GameLevel.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class LevelCover :
    public GameLevel
{
public:
	LevelCover(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window);

	// must call before clearing ECS 
	virtual void Restart() override;

	// Steps the game ahead by ms milliseconds
	virtual void step(float elapsed_ms) override;

	virtual void OnKey(int key, int, int action, int mod) override;

	virtual void OnMouseMove(double x, double y) override {}

	virtual void OnMouseButton(int button, int action, int mods) override;

private:
	Entity bgEntity; // backgrounds
};

