#pragma once

#include "image_provider.h"
#include "chapter.h"
#include "icon.h"

#include "include/core/SkPaint.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"

#include <chrono>
#include <vector>

class SkSurface;
class SkCanvas;

class SeekBar {
public:
    SeekBar(SkCanvas* canvas, int windowWidth, int windowHeight);

    void draw();
    void load(const std::chrono::time_point<std::chrono::steady_clock>& start);

    bool isMouseWithinBar(const double mouseX, const double mouseY) const;
    bool isMouseWithinIcons(const double mouseX, const double mouseY) const;
    void handleButtonClick(const double mouseX, const double mouseY);

    void updateCursorPosition(const double mouseX);
    void setCursorVisibility(const bool visible);
    void startCursorDragging();
    void stopCursorDragging();
    bool isCursorDragging() const;
    double getCursorX() const;
    double getCursorY() const;

    void setHoverForChapter(const double xpos);
    void resetHover();

    bool isLoading() const;

private:
    void drawFullBar();
    void drawDefaultBar();
    void drawIndeterminateLoading();
    void drawSeekBarDividedByChapters();
    void drawChapter(const Chapter& chapter, SkPaint& chapterPaint, const double startX, const double endX);
    void drawMarker(const Chapter& chapter, SkPaint& markerPaint, const double start);
    void drawIcons();
    void drawElapsedTime();
    void drawCursor();

    ImageProvider _imageProvider;

    sk_sp<SkFontMgr> _fontMgr;
    sk_sp<SkTypeface> _typeface;

    SkCanvas* _canvas;

    int _windowWidth;
    int _windowHeight;
    double _padding;
    double _width;
    double _height;
    double _cursorX;
    double _cursorY;
    double _currentTime;
    double _animationOffset;
    bool _isPlaying;
    bool _isMuted;
    bool _isCursorVisible; // Flag to track if the cursor should be visible
    bool _isCursorDragging; // Flag to track if the user is _isCursorDragging the cursor
    bool _isFileLoaded;
    bool _isIndeterminateLoading;

    std::chrono::time_point<std::chrono::steady_clock> _start;

    std::vector<Chapter> _chapters = {
        {.label = "Intro", .start = 0.0, .end = 0.2},
        {.label = "Main Topic", .start = 0.2, .end = 0.5},
        {.label = "Details", .start = 0.5, .end = 0.9},
        {.label = "Outro", .start = 0.9, .end = 1.0}
    };
    std::vector<Icon> _icons;
};
