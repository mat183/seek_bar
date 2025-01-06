#pragma once

#include "include/core/SkRefCnt.h"

#include <filesystem>

class SkImage;

class ImageProvider {
public:
    ImageProvider(const std::filesystem::path& parentDir);

    sk_sp<SkImage> playImg() const;
    sk_sp<SkImage> pauseImg() const;
    sk_sp<SkImage> skipImg() const;
    sk_sp<SkImage> volumeImg() const;
    sk_sp<SkImage> muteImg() const;

private:
    sk_sp<SkImage> _playImage = nullptr;
    sk_sp<SkImage> _pauseImage = nullptr;
    sk_sp<SkImage> _skipImage = nullptr;
    sk_sp<SkImage> _volumeImage = nullptr;
    sk_sp<SkImage> _muteImage = nullptr;
};
