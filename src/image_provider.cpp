#include "image_provider.h"

#include "include/core/SkImage.h"
#include "include/core/SkData.h"

ImageProvider::ImageProvider(const std::filesystem::path& parentDir)
    : _playImage{SkImages::DeferredFromEncodedData(
        SkData::MakeFromFileName((parentDir / "icons/play.png").c_str()))}
    , _pauseImage{SkImages::DeferredFromEncodedData(
        SkData::MakeFromFileName((parentDir / "icons/pause.png").c_str()))}
    , _skipImage{SkImages::DeferredFromEncodedData(
        SkData::MakeFromFileName((parentDir / "icons/skip.png").c_str()))}
    , _volumeImage{SkImages::DeferredFromEncodedData(
        SkData::MakeFromFileName((parentDir / "icons/volume.png").c_str()))}
    , _muteImage{SkImages::DeferredFromEncodedData(
        SkData::MakeFromFileName((parentDir / "icons/mute.png").c_str()))} {
    if (!_playImage || !_pauseImage || !_skipImage || !_volumeImage || !_muteImage) {
        throw std::runtime_error("Failed to create image from encoded data");
    }
}

sk_sp<SkImage> ImageProvider::playImg() const {
    return _playImage;
}

sk_sp<SkImage> ImageProvider::pauseImg() const {
    return _pauseImage;
}

sk_sp<SkImage> ImageProvider::skipImg() const {
    return _skipImage;
}

sk_sp<SkImage> ImageProvider::volumeImg() const {
    return _volumeImage;
}

sk_sp<SkImage> ImageProvider::muteImg() const {
    return _muteImage;
}
