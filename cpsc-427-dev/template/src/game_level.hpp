class GameLevel
{
public:
	int level;
	vec2 character_position; // TODO: for the future, maybe save character position
	// load level from file
	void loadlLevelFromFile(const char* file);
	// save level to file
	void saveLevelToFile(const char* file);
	// reset level
	void resetToInitialLevel(const char* file);
};