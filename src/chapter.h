#pragma once

#include <string>

struct Chapter {
    std::string label;
    double start;       // Start position (0.0 to 1.0, relative to seek bar)
    double end;         // End position (0.0 to 1.0, relative to seek bar)
    double width = 0.0;
    double height = 15.0;
    double mouseX = 0.0;
    bool isHovered = false;
};
