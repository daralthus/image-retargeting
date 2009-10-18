#pragma once

#include "RGB.h"
#include "Image.h"
#include "GaussianPyramid.h"

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> LoadImage(const std::string& path)
    {
        Image<PixelType> result;
        Convert(result, LoadImage<RGB8>(path));
        return result;
    }

    template<class PixelType>
    bool SaveImage(const Image<PixelType>& image, const std::string& path)
    {
        Image<RGB8> result;
        Convert(result, image);
        return SaveImage(result, path);
    }

    template<class PixelType>
    bool SaveGaussianPyramid(const GaussianPyramid<PixelType>& image, const std::string& path)
    {
        GaussianPyramid<RGB8> result;
        Convert(result, image);
        return SaveGaussianPyramid(result, path);
    }

    template<> extern const Image<RGB8> LoadImage(const std::string& path);
    template<> extern bool SaveImage(const Image<RGB8>& image, const std::string& path);
    template<> extern bool SaveGaussianPyramid(const GaussianPyramid<RGB8>& pyramid, const std::string& path);
}