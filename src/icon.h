#pragma once

#include "include/core/SkRefCnt.h"

#include <string>

class SkImage;

struct Icon {
    double x;
    double y;
    double width;
    double height;
    sk_sp<SkImage> image;
};
