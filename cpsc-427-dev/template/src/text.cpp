#include "text.hpp"
#include <gl3w.h>
// stlib
#include <chrono>

void TextUtility::DisplayText(std::string& string, int x, int y) {
    // Disable 2D texturing
    glDisable(GL_TEXTURE_2D);

    // Reenable 2D texturing
    glEnable(GL_TEXTURE_2D);
}