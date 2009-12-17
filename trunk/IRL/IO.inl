#include "IO.h"

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
    const ImageWithMask<PixelType> LoadImageWithMask(const std::string& path)
    {
        ImageWithMask<PixelType> result;
        Convert(result, LoadImageWithMask<RGB8>(path));
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
    bool SaveImage(const ImageWithMask<PixelType>& image, const std::string& path)
    {
        ImageWithMask<RGB8> result;
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
}