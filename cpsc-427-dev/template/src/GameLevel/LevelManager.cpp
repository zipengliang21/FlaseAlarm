#include "LevelManager.h"

#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

#include "LevelCover.h"
#include "LevelSelection.h"
#include "LevelTutorialPage.h"
#include "LevelPlay.h"

using namespace std;

LevelManager::LevelManager() :renderer(nullptr), window(nullptr), curLevel(nullptr), cur_bg_music(nullptr)
{
	playing_bg_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	assert(playing_bg_music);

	cover_bg_music = Mix_LoadMUS(audio_path("cover.wav").c_str());
	auto err=Mix_GetError();
	assert(cover_bg_music);
}

LevelManager::~LevelManager()
{
	if (playing_bg_music != nullptr)
		Mix_FreeMusic(playing_bg_music);

	if (cover_bg_music != nullptr)
		Mix_FreeMusic(cover_bg_music);
}

void LevelManager::init(RenderSystem *renderer, GLFWwindow *window)
{
	// set renderer
	this->renderer = renderer;

	// 
	gameStateEntity = createGameState();

	// load level
	levelCover.reset(new LevelCover(renderer, this, window));
	levelTutorialPage.reset(new LevelTutorialPage(renderer, this, window));
	levelSelection.reset(new LevelSelection(renderer, this, window));
	levelPlay.reset(new LevelPlay(renderer, this, window));

	GoCover();
	//GoLevelSelection();
}

void LevelManager::step(float elapsed_ms)
{
	curLevel->step(elapsed_ms);
}

void LevelManager::handle_collisions()
{
	curLevel->handle_collisions();

	// don't forget to clear collisions
	assert(registry.collisions.size() == 0);
}

void LevelManager::GoCover()
{
	if (cur_bg_music != cover_bg_music)
	{
		Mix_PlayMusic(cover_bg_music, -1);
		cur_bg_music = cover_bg_music;
	}

	ResetRenderer();
	curLevel = levelCover.get();
	curLevel->Restart();
}

void LevelManager::GoLevelSelection()
{
	// Playing background music indefinitely
	if (cur_bg_music != playing_bg_music)
	{
		Mix_PlayMusic(playing_bg_music, -1);
		cur_bg_music = playing_bg_music;
	}

	ResetRenderer();
	curLevel = levelSelection.get();
	curLevel->Restart();
}

void LevelManager::GoTutorialPage()
{
	ResetRenderer();
	curLevel = levelTutorialPage.get();
	curLevel->Restart();
}

void LevelManager::GoPlay()
{
	ResetRenderer();
	curLevel = levelPlay.get();
	curLevel->Restart();
}

void LevelManager::ResetRenderer()
{
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset Camera
	renderer->viewMatrix = mat3(1.0f);

	// Reset the trap effect
	while (registry.trappables.entities.size() > 0)
		registry.remove_all_components_of(registry.trappables.entities.back());


	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	//while (registry.collisions.entities.size() > 0)
	//	registry.remove_all_components_of(registry.collisions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
}

void LevelManager::OnKey(int key, int scancode, int action, int mod)
{
	curLevel->OnKey(key, scancode, action, mod);
}

void LevelManager::OnMouseMove(double x, double y)
{
	curLevel->OnMouseMove(x, y);
}

void LevelManager::OnMouseButton(int button, int action, int mods)
{
	curLevel->OnMouseButton(button, action, mods);
}
