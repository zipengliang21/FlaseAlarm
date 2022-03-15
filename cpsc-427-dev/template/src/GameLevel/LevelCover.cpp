#include "LevelCover.h"

#include "LevelManager.h"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

using namespace std;

LevelCover::LevelCover(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window) :GameLevel(renderer, manager, window)
{
}

void LevelCover::Restart()
{
	bgEntity = createMovie(renderer, { window_width_px / 2.0,window_height_px / 2.0 }, { window_width_px, window_height_px }, {
		TEXTURE_ASSET_ID::COVER0,
		TEXTURE_ASSET_ID::COVER1,
		TEXTURE_ASSET_ID::COVER2,
		TEXTURE_ASSET_ID::COVER3
		},3);// every 3 seconds switch a picture

	//createUIBox(renderer, { window_width_px / 2.0,window_height_px*0.2 }, { 800,200 }, TEXTURE_ASSET_ID::TITLE, "");
	//createUIBox(renderer, { window_width_px / 2.0,window_height_px*0.8 }, { 800,200 }, TEXTURE_ASSET_ID::PRESS_ANY, "");
}

void LevelCover::step(float elapsed_ms)
{
	Movie &bg = registry.movies.get(bgEntity);
	TEXTURE_ASSET_ID texId=bg.GetTexId(glfwGetTime()); // refresh texture id

	registry.renderRequests.get(bgEntity).changeTexture(texId); // do change
	//cout << (int)texId << endl;
}

void LevelCover::OnKey(int key, int, int action, int mod)
{
	if (action == GLFW_PRESS)
	{
		manager->GoLevelSelection();
	}
}

void LevelCover::OnMouseButton(int button, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		//manager->GoLevelSelection();
	}
}
