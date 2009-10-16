#pragma once

#include "RGB.h"
#include "Image.h"
#include "GaussianPyramid.h"

namespace IRL
{
    extern const Image<RGB8> LoadImage(const std::string& path);

    extern bool SaveImage(const Image<RGB8>& image, const std::string& path);
    extern bool SaveGaussianPyramid(const GaussianPyramid<RGB8>& pyramid, const std::string& path);
}