// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>

// internal
#include "physics_system.hpp"

// maximum levels of game we are going to provide
const int MAX_LEVEL = 6;
// file path to save level information to
const std::string UNLOCKED_LEVEL_FILE_PATH = text_path("unlockedLevel.txt");

class GameLevel
{
public:
	int currLevel;
	int unlockedLevel;
	

	GameLevel() {
		currLevel = -1;
		unlockedLevel = 1;
	}

	GameLevel(bool loadFromPrevState) {
		currLevel = -1;
		if (loadFromPrevState) {
			loadlLevelFromFile(); // TODO: create a file that do this
		}
		else {
			unlockedLevel = 1; // level uses 1-based index
		}
	}
	vec2 character_position; // TODO: for the future, maybe save character position
	// load level from file
	void loadlLevelFromFile();
	// save level to file
	void saveLevelToFile();
	// reset level
	void resetToInitialLevel();
};


class GameState
{
public:
	GameState() {
		state = GAME_STATE::LEVEL_SELECTION;
		gameLevel = GameLevel(false); // TODO: change to true after save/load is implemented
	}
	enum class GAME_STATE {
		LEVEL_SELECTION = 0,
		LEVEL_SELECTED = 1,
		GAME_STATE_COUNT = LEVEL_SELECTED + 1
	};

	const int game_state_count = (int)GAME_STATE::GAME_STATE_COUNT;

	GAME_STATE state;

	GameLevel gameLevel;
};