#include "LevelSelection.h"

#include "LevelManager.h"

#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"


LevelSelection::LevelSelection(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window) :GameLevel(renderer, manager, window)
{
	Restart();
}

void LevelSelection::Restart()
{
	// if we are in level selection menu

	// Create Level Selection
	float paddingFactor = 8;

	float w = window_width_px;
	float h = window_height_px;

	createUIBox(renderer, { w / 2.0,h / 2.0 }, { w,h }, TEXTURE_ASSET_ID::SELECTION_BG, "");

	createUIBox(renderer, { w / 2, 1 * (h - 50) / paddingFactor }, {BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT}, TEXTURE_ASSET_ID::TUTORIAL_BUTTON,  "show tutorial");
	createUIBox(renderer, { w / 2, 2 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(1), "level1");
	createUIBox(renderer, { w / 2, 3 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(2), "level2");
	createUIBox(renderer, { w / 2, 4 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(3), "level3");
	createUIBox(renderer, { w / 2, 5 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(4),"level4");
	createUIBox(renderer, { w / 2, 6 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(5), "level5");
	createUIBox(renderer, { w / 2, 7 * (h - 50) / paddingFactor }, { BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT }, getTextureIDOfLevelButton(6), "level6");


	registry.gameStates.get(manager->gameStateEntity).SetCurrentLevelIndex(-1);
	renderer->useMask = false;
}

void LevelSelection::OnKey(int key, int, int action, int mod)
{
	// resume previous state if user have previous state
	if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
		GameState *gameState = &registry.gameStates.get(manager->gameStateEntity);
		if (gameState->loadGameState()) {
			manager->GoPlay();
		}
		
	}

}

void LevelSelection::OnMouseMove(double x, double y)
{
	// hover and box be bigger
	for (auto &c : registry.clickables.components) 
	{
		if (inRange(c.position, c.width, c.height, x, y)) 
		{
			// not work now
			//registry.motions.get(c.);
			c.height=BUTTON_BB_HEIGHT * 1.1f;
		}
		else
		{
			c.height = BUTTON_BB_HEIGHT;
		}
	}
}


void LevelSelection::OnMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		//std::cout << "Left Button Released" << std::endl;

		double cursorX, cursorY;
		glfwGetCursorPos(window, &cursorX, &cursorY);

		// clickable component that we clicked on
		Clickable *clickedClickable = findClickedButton(cursorX, cursorY);
		if (clickedClickable != NULL) {
			// get button action
			std::string buttonAction = clickedClickable->buttonAction;

			// if we are on level selection page
			//std::cout << "Selecting Levls" << std::endl;

			// check if user clicked on the level selection button
			int new_level = changeLevel(buttonAction);
			if (new_level != -1) {
				//std::cout << "new_level is: " << new_level << std::endl;
				manager->GoPlay();
				return; // we changed the level
			}

			// check if user clicked tutorial button
			if (buttonAction == "show tutorial") {
				manager->GoTutorialPage();
				return;
			}
		}

	}
}

TEXTURE_ASSET_ID LevelSelection::getTextureIDOfLevelButton(int level) {

	// TODO: what the magic number is ??? very bad
	int unlocked_id = (int)TEXTURE_ASSET_ID::LEVEL1 + (level - 1) * 2; // skip other texture + skip prev levels + current level
	int locked_id = unlocked_id + 1;
	TEXTURE_ASSET_ID texture_id;
	GameState *gameState = &registry.gameStates.get(manager->gameStateEntity);
	if (gameState->GetUnlockedLevel() >= level) {
		texture_id = static_cast<TEXTURE_ASSET_ID>(unlocked_id);
	}
	else {
		texture_id = static_cast<TEXTURE_ASSET_ID>(locked_id);
	}
	return texture_id;

}


// return -1 if level unchanged
// return the new level if success
int LevelSelection::changeLevel(std::string buttonAction) {
	int switchToLevel = -1;
	if (buttonAction == "level1") {
		switchToLevel = 1;
	}
	else if (buttonAction == "level2") {
		switchToLevel = 2;
	}
	else if (buttonAction == "level3") {
		switchToLevel = 3;
	}
	else if (buttonAction == "level4") {
		switchToLevel = 4;
	}
	else if (buttonAction == "level5") {
		switchToLevel = 5;
	}
	else if (buttonAction == "level6") {
		switchToLevel = 6;
	}

	if (switchToLevel != -1) {
		GameState *gameState = &registry.gameStates.get(manager->gameStateEntity);
		if (switchToLevel <= gameState->GetUnlockedLevel()) {
			gameState->SetCurrentLevelIndex(switchToLevel);
		}
		else {
			//std::cout << "Tried to access locked level, failed" << std::endl;
			switchToLevel = -1;
		}
	}

	return switchToLevel;

	// change level to switchToLevel

}