#include "seek_bar.h"
#include "utils.h"

#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#include <iostream>
#include <fstream>
#include <filesystem>

constexpr int defaultLoadingTime = 5; // seconds
constexpr int defaultFontSize = 20;
constexpr int defaultMarkerWidth = 5;
constexpr int defaultCursorRadius = 20;

const double seekBarDuration = 600.0;

const std::filesystem::path currentDir = std::filesystem::current_path();
const std::filesystem::path parentDir = currentDir.parent_path();
const std::filesystem::path absoluteParentDir = std::filesystem::absolute(parentDir);
const std::filesystem::path defaultFontPath = absoluteParentDir / "fonts/Roboto-Regular.ttf";

SeekBar::SeekBar(SkCanvas* canvas, int windowWidth, int windowHeight)
    : _imageProvider{absoluteParentDir}
    , _fontMgr{SkFontMgr_New_FontConfig(nullptr)}
    , _typeface{_fontMgr->makeFromFile(defaultFontPath.c_str())}
    , _canvas{canvas}
    , _windowWidth{windowWidth}
    , _windowHeight{windowHeight}
    , _padding{50.0}
    , _width{_windowWidth - 2 * _padding}
    , _height{15.0}
    , _cursorX{50}
    , _cursorY{_windowHeight / 2.0}
    , _currentTime{0.0}
    , _animationOffset{0.0}
    , _isPlaying{false}
    , _isMuted{false}
    , _isCursorVisible{false}
    , _isCursorDragging{false}
    , _isFileLoaded{false}
    , _isIndeterminateLoading{false} {
    if (!_typeface) {
        throw std::runtime_error("Failed to load typeface from file");
    }
    _canvas->scale(1, -1);
    _canvas->translate(0, -_canvas->getBaseLayerSize().height());
}

void SeekBar::draw() {
    _canvas->clear(SK_ColorWHITE);

    if (_isFileLoaded) {
        drawFullBar();
    } else {
        drawDefaultBar();
    }
}

void SeekBar::drawFullBar() {
    if (_isIndeterminateLoading) {
        drawIndeterminateLoading();
    } else {
        drawSeekBarDividedByChapters();
        drawIcons();
        drawElapsedTime();
        if (_isCursorVisible) {
            drawCursor();
        }
    }
}

void SeekBar::drawDefaultBar() {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    SkRect unfilledRect = SkRect::MakeXYWH(
        _padding,
        _windowHeight / 2 - _height / 2,
        _width,
        _height);
    _canvas->drawRect(unfilledRect, paint);
}

void SeekBar::drawIndeterminateLoading() {
    _animationOffset += 0.01;

    if (_animationOffset > 1.0) {
        _animationOffset = 0.0;
    }

    constexpr double segmentWidth = 250.0;
    const double offset = _animationOffset * (_windowWidth - 350.0);

    SkPaint animation;
    animation.setColor(SK_ColorGRAY);
    SkRect unfilledRect = SkRect::MakeXYWH(
        _padding,
        _windowHeight / 2 - _height / 2,
        _width,
        _height);
    _canvas->drawRect(unfilledRect, animation);

    animation.setColor(SK_ColorRED);
    animation.setStyle(SkPaint::kFill_Style);
    SkRect animatedRect = SkRect::MakeXYWH(
        _padding + offset,
        _windowHeight / 2 - _height / 2,
        segmentWidth,
        _height);
    _canvas->drawRect(animatedRect, animation);

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - _start).count() >= defaultLoadingTime) {
        _animationOffset = 0.0f;
        _isIndeterminateLoading = false;
        std::cout << "Loading file completed!" << std::endl;
    }
}

void SeekBar::drawSeekBarDividedByChapters() {
    SkPaint markerPaint;
    markerPaint.setColor(SK_ColorWHITE);
    markerPaint.setStyle(SkPaint::kFill_Style);

    SkPaint labelPaint;
    labelPaint.setColor(SK_ColorBLACK);
    labelPaint.setAntiAlias(true);

    SkFont font{_typeface, defaultFontSize};

    SkPaint chapterPaint;
    for (const auto& chapter : _chapters) {
        const double startX = _padding + _width  * chapter.start;
        const double endX = _padding + _width * chapter.end;

        drawChapter(chapter, chapterPaint, startX, endX);
        double markerStart = &chapter == &(_chapters.front()) ? endX : startX;
        drawMarker(chapter, markerPaint, markerStart);

        if (chapter.isHovered) {
            SkRect chapterBounds;
            font.measureText(chapter.label.c_str(), chapter.label.size(), SkTextEncoding::kUTF8, &chapterBounds);
            float chapterWidth = chapterBounds.width();

            _canvas->drawSimpleText(
                chapter.label.c_str(),
                chapter.label.size(),
                SkTextEncoding::kUTF8,
                chapter.mouseX - (chapterWidth / 2),
                (_windowHeight / 2 - chapter.height / 2) - 40,
                font,
                labelPaint);

            double timeAtCursor = ((chapter.mouseX - _padding) / _width) * seekBarDuration;
            std::string timeLabel = formatTime(timeAtCursor);

            SkRect timeBounds;
            font.measureText(timeLabel.c_str(), timeLabel.length(), SkTextEncoding::kUTF8, &timeBounds);
            float timeWidth = timeBounds.width();

            _canvas->drawSimpleText(
                timeLabel.c_str(),
                timeLabel.size(),
                SkTextEncoding::kUTF8,
                chapter.mouseX - (timeWidth / 2),
                (_windowHeight / 2 - chapter.height / 2) - 20,
                font,
                labelPaint);
        }
    }
}

void SeekBar::drawChapter(const Chapter& chapter, SkPaint& chapterPaint, const double startX, const double endX) {
    chapterPaint.setColor(SK_ColorRED);
    SkRect filledRect = SkRect::MakeXYWH(
        startX,
        _windowHeight / 2 - chapter.height / 2,
        std::max(0.0, std::min(_cursorX - startX, chapter.width)),
        chapter.height);
    _canvas->drawRect(filledRect, chapterPaint);

    chapterPaint.setColor(SK_ColorGRAY);
    SkRect unfilledRect = SkRect::MakeXYWH(
        std::min(endX, std::max(_cursorX, startX)),
        _windowHeight / 2 - chapter.height / 2,
        std::max(0.0, std::min(endX - _cursorX, chapter.width)),
        chapter.height);
    _canvas->drawRect(unfilledRect, chapterPaint);
}

void SeekBar::drawMarker(const Chapter& chapter, SkPaint& markerPaint, const double start) {
    SkRect marker = SkRect::MakeXYWH(
        start,
        _windowHeight / 2 - chapter.height / 2,
        defaultMarkerWidth,
        chapter.height);
    _canvas->drawRect(marker, markerPaint);
}

void SeekBar::drawIcons() {
    for (const auto& icon : _icons) {
        const double imageX = icon.x + (icon.width - icon.image->width()) / 2.0;
        const double imageY = icon.y + (icon.height - icon.image->height()) / 2.0;
        _canvas->drawImage(icon.image, imageX, imageY);
    }
}

void SeekBar::drawElapsedTime() {
    SkPaint timePaint;
    timePaint.setColor(SK_ColorBLACK);
    timePaint.setAntiAlias(true);

    std::string elapsedTime = formatTime(_currentTime);
    std::string totalTime = formatTime(seekBarDuration);
    std::string timeText = elapsedTime + " / " + totalTime;

    SkFont font{_typeface, defaultFontSize};

    _canvas->drawSimpleText(
        timeText.c_str(),
        timeText.size(),
        SkTextEncoding::kUTF8,
        _padding + 220,
        _cursorY + 60,
        font,
        timePaint);
}

void SeekBar::drawCursor() {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    _canvas->drawCircle(_cursorX, _cursorY, defaultCursorRadius, paint);
}

void SeekBar::load(const std::chrono::time_point<std::chrono::steady_clock>& start) {
    _cursorX = _padding;
    _currentTime = 0.0;
    _animationOffset = 0.0;
    _start = start;
    _isPlaying = false;
    _isMuted = false;
    _isFileLoaded = true;
    _isIndeterminateLoading = true;

    for (auto& chapter : _chapters) {
        chapter.width = (chapter.end - chapter.start) * _width;
    }

    _icons = std::vector<Icon>{
        {_padding, _cursorY + 30, 50, 50, _imageProvider.playImg()},
        {_padding + 70, _cursorY + 30, 50, 50, _imageProvider.skipImg()},
        {_padding + 140, _cursorY + 30, 50, 50, _imageProvider.volumeImg()}
    };
}

bool SeekBar::isMouseWithinBar(const double mouseX, const double mouseY) const {
    return mouseX >= _padding && mouseX <= (_windowWidth - _padding)
        && mouseY >= (_windowHeight / 2 - 20) && mouseY <= (_windowHeight / 2 + 20);
}

bool SeekBar::isMouseWithinIcons(const double mouseX, const double mouseY) const {
    for (const auto& icon : _icons) {
        if (!_isIndeterminateLoading &&
            mouseX >= icon.x && mouseX <= icon.x + icon.width &&
            mouseY >= icon.y && mouseY <= icon.y + icon.height) {
            return true;
        }
    }

    return false;
}

void SeekBar::handleButtonClick(const double mouseX, const double mouseY) {
    for (size_t i = 0; i < _icons.size(); ++i) {
        const auto& icon = _icons[i];

        if (mouseX >= icon.x && mouseX <= icon.x + icon.width &&
            mouseY >= icon.y && mouseY <= icon.y + icon.height) {
            if (i == 0) {
                _isPlaying = !_isPlaying;
                _icons[0].image = _isPlaying ? _imageProvider.pauseImg() : _imageProvider.playImg();
                std::cout << (_isPlaying ? "Play" : "Pause") << " button clicked" << std::endl;
            } else if (i == 1) {
                std::cout << "Skip button clicked" << std::endl;
            } else if (i == 2) {
                _isMuted = !_isMuted;
                _icons[2].image = _isMuted ? _imageProvider.muteImg() : _imageProvider.volumeImg();
                std::cout << "Mute button clicked" << std::endl;
            }
            break;
        }
    }
}

void SeekBar::updateCursorPosition(const double mouseX) {
    if (!_isIndeterminateLoading && _isFileLoaded) {
        _cursorX = std::max(_padding, std::min(_windowWidth - _padding, mouseX));
        _currentTime = ((_cursorX - _padding) / _width) * seekBarDuration;
    }
}

void SeekBar::setCursorVisibility(const bool visible) {
    _isCursorVisible = visible;
}

void SeekBar::startCursorDragging() {
    _isCursorDragging = true;
}

void SeekBar::stopCursorDragging() {
    _isCursorDragging = false;
}

bool SeekBar::isCursorDragging() const {
    return _isCursorDragging;
}

double SeekBar::getCursorX() const {
    return _cursorX;
}

double SeekBar::getCursorY() const {
    return _cursorY;
}

void SeekBar::setHoverForChapter(const double xpos) {
    for (auto& chapter : _chapters) {
        double startX = _padding + _width  * chapter.start;
        double endX = _padding + _width * chapter.end;
        chapter.mouseX = xpos;
        chapter.isHovered = xpos >= startX && xpos <= endX;
        chapter.height = chapter.isHovered ? 20.0 : 15.0;
    }
}

void SeekBar::resetHover() {
    for (auto& chapter : _chapters) {
        chapter.isHovered = false;
        chapter.height = 15.0;
    }
}

bool SeekBar::isLoading() const {
    return _isIndeterminateLoading;
}
