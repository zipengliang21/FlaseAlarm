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

// internal

// maximum levels of game we are going to provide
extern const int MAX_LEVEL;

class GameState
{
public:
	using GameMap = std::vector<std::vector<char>>;

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

private:
	int currLevelIndex;
	int unlockedLevel;
	std::map<int, GameMap> levelMaps; // key = level index; value = level map;

	void LoadLevel(int levelIndex);

	void LoadUnlockedLevel();

	void SaveUnlockedLevel();
};