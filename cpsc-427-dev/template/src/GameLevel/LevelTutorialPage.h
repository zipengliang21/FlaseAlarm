#pragma once
#include "GameLevel.h"

class LevelTutorialPage :
    public GameLevel
{
public:
	LevelTutorialPage(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window);

	// must call before clearing ECS 
	virtual void Restart() override;

	// Check for collisions
	virtual void handle_collisions() override;

	virtual void OnKey(int key, int, int action, int mod) override;

	virtual void OnMouseButton(int button, int action, int mods) override;

private:
};

