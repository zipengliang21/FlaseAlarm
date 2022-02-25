// stlib
#include <glm/glm.hpp>

#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// internal

// maximum levels of game we are going to provide
extern const int MAX_LEVEL;
// file path to save level information to
extern const std::string UNLOCKED_LEVEL_FILE_PATH;

class GameLevel
{
public:
	int currLevel;
	int unlockedLevel;
	
	std::vector<std::vector<char>> levelMap; // the map of this level

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
	glm::vec2 character_position; // TODO: for the future, maybe save character position
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

	GameState & operator=(const GameState & other)
	{
		//game_state_count = other.game_state_count;
		state = other.state;
		gameLevel = other.gameLevel;
		return *this;
	}

	enum class GAME_STATE {
		LEVEL_SELECTION = 0,
		LEVEL_SELECTED = 1,
		TUTORIAL_PAGE = 2,
		GAME_STATE_COUNT = TUTORIAL_PAGE + 1
	};

	const int game_state_count = (int)GAME_STATE::GAME_STATE_COUNT;

	GAME_STATE state;

	GameLevel gameLevel;
};