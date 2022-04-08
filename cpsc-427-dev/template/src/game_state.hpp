#pragma once

// stlib
#include <glm/glm.hpp>

#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>


// json library
#include <json.hpp>
using json = nlohmann::json;

// #include "tiny_ecs_registry.hpp"
#include "common.hpp"

// maximum levels of game we are going to provide
extern const int MAX_LEVEL;
// file path to save level information to
const std::string UNLOCKED_LEVEL_FILE_PATH = text_path("unlockedLevel.txt");
const std::string GameStatus_JSON_FILE_PATH = json_path("game_status.json");

class GameState
{
public:
	using GameMap = std::vector<std::vector<char>>;
	int savedState;
	json jsonObject; // json variable for save and load

	// load all the level files and unlocked level list when create
	GameState();

	// returns the current map
	const GameMap &GetCurrentMap()const;

	// returns the pixel size of the current map
	glm::vec2 GetMapPixelSize() const;

	int GetUnlockedLevel() const;

	void WinAtLevel(int winLevelIndex);

	bool AtValidLevel() const;

	int GetCurrentLevelIndex() const;

	void SetCurrentLevelIndex(int index);

	void saveGameState(Entity& player);

	void getSavedPlayerMotion();

	bool loadGameState();

	int getHighestPoint(int levelIndex);

	void saveHighestPoint(int levelIndex, int currentPoint);
	

private:
	int currLevelIndex;
	int unlockedLevel;
	
	
	
	std::map<int, GameMap> levelMaps; // key = level index; value = level map;

	void LoadLevel(int levelIndex);

	void LoadUnlockedLevel();

	void saveUnlockedLevel();

	
};