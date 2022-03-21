// internal
#include "game_state.hpp"

#include "common.hpp"

// maximum levels of game we are going to provide
const int MAX_LEVEL = 6;

// file path to save level information to
const std::string UNLOCKED_LEVEL_FILE_PATH = text_path("unlockedLevel.txt");

GameState::GameState() :currLevelIndex(-1)
{
	LoadUnlockedLevel();
	LoadLevel(1);
	LoadLevel(2);
	LoadLevel(3);
	LoadLevel(4);
	LoadLevel(5);
	LoadLevel(6);
}

const GameState::GameMap &GameState::GetCurrentMap() const
{
	return levelMaps.at(currLevelIndex);
}

int GameState::GetUnlockedLevel() const
{
	return unlockedLevel;
}

void GameState::WinAtLevel(int winLevelIndex)
{
	// check if we are not at maximum level + we are playing the last unlocked level
	if (unlockedLevel < MAX_LEVEL && unlockedLevel == currLevelIndex) {

		unlockedLevel += 1;
		SaveUnlockedLevel();
	}
}

int GameState::GetCurrentLevelIndex() const
{
	return currLevelIndex;
}

void GameState::SetCurrentLevelIndex(int index)
{
	assert(levelMaps.find(index) != levelMaps.end());
	currLevelIndex = index;
}

void GameState::LoadLevel(int levelIndex)
{
	std::ifstream in(level_map_path("level" + std::to_string(levelIndex) + ".txt"));
	std::string map_row;

	std::vector<std::vector<char>> temp_map;


	while (std::getline(in, map_row)) {
		std::vector<char> charVector(map_row.begin(), map_row.end());
		temp_map.push_back(charVector);
	}

	// add for holding
	levelMaps[levelIndex] = temp_map;
}

void GameState::LoadUnlockedLevel()
{
	std::string filename(UNLOCKED_LEVEL_FILE_PATH);
	int levelInFile = -1;
	std::ifstream input_file(filename);
	if (!input_file.is_open()) {
		std::cout << "Cannot open " << UNLOCKED_LEVEL_FILE_PATH << std::endl;
	}
	std::cout << "levelInFile before is " << levelInFile << std::endl;

	while (input_file >> levelInFile)
	{
		std::cout << "levelInFile is " << levelInFile << std::endl;
	}

	input_file.close();
	unlockedLevel = levelInFile;
	std::cout << "unlockedLevel is " << unlockedLevel << std::endl;

}

void GameState::SaveUnlockedLevel()
{
	std::ofstream unlockedLevelFile;
	unlockedLevelFile.open(UNLOCKED_LEVEL_FILE_PATH);
	std::cout << "before saving" << std::endl;

	unlockedLevelFile << unlockedLevel << "\n";
	std::cout << unlockedLevel << std::endl;

	std::cout << "after saving" << std::endl;
	unlockedLevelFile.close();
}
