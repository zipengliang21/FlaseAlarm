#include <string>
#include <iostream>


class TextUtility
{
public:
	// display text to the screen at (x, y) position
	// can be extended for font size, color in the future...
	void DisplayText(std::string & string, int x, int y);
};