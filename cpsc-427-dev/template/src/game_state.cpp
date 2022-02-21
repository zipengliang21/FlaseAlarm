// internal
#include "game_state.hpp"

// load level from file
void GameLevel::loadlLevelFromFile() {
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
// save level to file
void GameLevel::saveLevelToFile() {
	std::ofstream unlockedLevelFile;
	unlockedLevelFile.open(UNLOCKED_LEVEL_FILE_PATH);
	std::cout << "before saving" << std::endl;
	unlockedLevelFile << unlockedLevel << "\n";
	std::cout << unlockedLevel << std::endl;
	std::cout << "after saving" << std::endl;
	unlockedLevelFile.close();
}
// reset level
void GameLevel::resetToInitialLevel() {
	// game level is 1-based index
	currLevel = 1;
	unlockedLevel = 1;
}