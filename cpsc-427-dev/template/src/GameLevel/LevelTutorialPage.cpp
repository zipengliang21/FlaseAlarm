#include "LevelTutorialPage.h"

#include "world_system.hpp"
#include "tiny_ecs_registry.hpp"

LevelTutorialPage::LevelTutorialPage(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window):GameLevel(renderer,manager,window)
{
	Restart();
}

void LevelTutorialPage::Restart()
{
	// display tutorial image
	createTextBox(renderer, { window_width_px/2.0,window_height_px/2.0 }, TEXTURE_ASSET_ID::TUTORIAL_CONTENT, TUTORIAL_BB_WIDTH, TUTORIAL_BB_HEIGHT, "none");
}

void LevelTutorialPage::handle_collisions()
{
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void LevelTutorialPage::OnKey(int key, int, int action, int mod)
{
	// std::cout << "On key for tutorial page is pressed" << std::endl;
	if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
		// go back to menu
		// std::cout << "M key pressed and released" << std::endl;
		manager->GoLevelSelection();
	}
}

void LevelTutorialPage::OnMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		manager->GoLevelSelection();
	}

}
