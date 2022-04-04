#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"

#include <memory>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class GameLevel;
class LevelCover;
class LevelTutorialPage;
class LevelSelection;
class LevelPlay;

class RenderSystem;

class LevelManager
{
public:
	Entity gameStateEntity;
	std::unique_ptr<LevelCover> levelCover;
	std::unique_ptr<LevelTutorialPage> levelTutorialPage;
	std::unique_ptr<LevelSelection> levelSelection;
	std::unique_ptr<LevelPlay> levelPlay;

	LevelManager();

	~LevelManager();

	void init(RenderSystem *renderer, GLFWwindow *window);

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	void GoCover();
	void GoLevelSelection();
	void GoTutorialPage();
	void GoPlay();

	// Input functions
	void OnKey(int key, int scancode, int action, int mod);

	void OnMouseMove(double x, double y);

	void OnMouseButton(int button, int action, int mods);

	void reloadState();

private:
	RenderSystem *renderer; 
	GLFWwindow *window;

	GameLevel *curLevel;

	

	Mix_Music *cover_bg_music;
	Mix_Music *playing_bg_music;
	Mix_Music *cur_bg_music; // don't release
	
	void ResetRenderer();

};

