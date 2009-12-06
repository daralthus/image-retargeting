#pragma once

#include "RGB.h"
#include "Image.h"
#include "GaussianPyramid.h"

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> LoadImage(const std::string& path);

    template<class PixelType>
    bool SaveImage(const Image<PixelType>& image, const std::string& path);

    template<class PixelType>
    bool SaveGaussianPyramid(const GaussianPyramid<PixelType>& image, const std::string& path);

    // Concrete implementations for RGB8
    template<> extern const Image<RGB8> LoadImage(const std::string& path);
    template<> extern bool SaveImage(const Image<RGB8>& image, const std::string& path);
    template<> extern bool SaveGaussianPyramid(const GaussianPyramid<RGB8>& pyramid, const std::string& path);
}

#include "IO.inl"