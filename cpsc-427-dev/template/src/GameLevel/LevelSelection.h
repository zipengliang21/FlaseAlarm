#pragma once

#include "GameLevel.h"

#include "components.hpp"

class LevelSelection:public GameLevel
{
public:
	LevelSelection(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window);

	// must call before clearing ECS 
	virtual void Restart() override;

	virtual void OnKey(int key, int, int action, int mod) override;

	virtual void OnMouseMove(double x, double y) override;

	virtual void OnMouseButton(int button, int action, int mods) override;

private:
	// get texture id
	TEXTURE_ASSET_ID getTextureIDOfLevelButton(int level);

	// change level
	int changeLevel(std::string buttonAction);
};

