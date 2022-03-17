#include "GameLevel.h"

#include "tiny_ecs_registry.hpp"

void GameLevel::handle_collisions()
{
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


bool inRange(vec2 buttonPos, int buttonWidth, int buttonHeight, double cursorX, double cursorY)
{
	float buttonLeft = buttonPos.x - (buttonWidth / 2);
	float buttonRight = buttonPos.x + (buttonWidth / 2);
	float buttonTop = buttonPos.y - (buttonHeight / 2);
	float buttonBot = buttonPos.y + (buttonHeight / 2);
	float normalizedCursorX = cursorX; // TODO: this is related to background size UPDATE: this has been well.
	float normalizedCursorY = cursorY;
	bool xInRange = (buttonLeft <= normalizedCursorX) && (buttonRight >= normalizedCursorX);
	bool yInRange = (buttonTop <= normalizedCursorY) && (buttonBot >= normalizedCursorY);
	//std::cout << "buttonPos " << buttonPos[0] << " " << buttonPos[1] << std::endl;
	//std::cout << "buttonWidth " << buttonWidth << std::endl;
	//std::cout << "buttonHeight " << buttonHeight << std::endl;
	//std::cout << "normalizedCursorX " << normalizedCursorX << std::endl;
	//std::cout << "normalizedCursorY " << normalizedCursorY << std::endl;
	//std::cout << "buttonTop " << buttonTop << std::endl;
	//std::cout << "buttonBot " << buttonBot << std::endl;
	//std::cout << "xInRange " << xInRange << std::endl;
	//std::cout << "yInRange " << yInRange << std::endl;
	return xInRange && yInRange;
}


Clickable *findClickedButton(double cursorX, double cursorY)
{
	// uses cursorX and cursorY to see if it is in range of any button
	//std::cout << "findClickedButton" << std::endl;
	for (auto &c : registry.clickables.components) {
		//std::cout << "----check button range ----" << std::endl;
		if (inRange(c.position, c.width, c.height, cursorX, cursorY)) {
			//std::cout << "Found Clicked Button" << std::endl;

			return &c; // TODO: does this work?
		}
	}
	return NULL;
}

bool findClickedEntity(double cursorX, double cursorY, Entity &result)
{
	int sz = registry.clickables.components.size();
	for (int i = 0; i < sz;++i) 
	{
		//std::cout << "----check button range ----" << std::endl;
		auto &c = registry.clickables.components[i];
		if (inRange(c.position, c.width, c.height, cursorX, cursorY)) {
			//std::cout << "Found Clicked Button" << std::endl;

			result=registry.clickables.entities[i];
			return true;
		}
	}
	return false;
}