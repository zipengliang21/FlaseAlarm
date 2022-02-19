// stlib
#include <cassert>
#include <sstream>

// internal
#include "physics_system.hpp"

class GameLevel
{
public:
	int currLevel;
	int unlockedLevel;

	GameLevel() {
		unlockedLevel = 1;
	}

	GameLevel(bool loadFromPrevState) {
		if (loadFromPrevState) {
			loadlLevelFromFile("this is fake for now"); // TODO: create a file that do this
		}
		else {
			unlockedLevel = 1; // level uses 1-based index
		}
	}
	vec2 character_position; // TODO: for the future, maybe save character position
	// load level from file
	void loadlLevelFromFile(const char* file);
	// save level to file
	void saveLevelToFile(const char* file);
	// reset level
	void resetToInitialLevel(const char* file);
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
		LEVEL1_SELECTED = 1,
		GAME_STATE_COUNT = LEVEL1_SELECTED + 1
	};

	const int game_state_count = (int)GAME_STATE::GAME_STATE_COUNT;

	GAME_STATE state;

	GameLevel gameLevel;
};