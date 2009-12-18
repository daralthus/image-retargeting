#pragma once

#include "RGB.h"
#include "Image.h"
#include "GaussianPyramid.h"

#ifdef IRL_USE_QT
#include <QtGui/QImage>
#else
#error Implement ImageIO without Qt.
#endif

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> LoadImage(const std::string& path);

    template<class PixelType>
    const Image<PixelType> LoadFromQImage(const QImage& path);

    template<class PixelType>
    const ImageWithMask<PixelType> LoadImageWithMask(const std::string& path);

    template<class PixelType>
    bool SaveImage(const Image<PixelType>& image, const std::string& path);

    template<class PixelType>
    QImage SaveToQImage(const Image<PixelType>& image);

    template<class PixelType>
    bool SaveImage(const ImageWithMask<PixelType>& image, const std::string& path);

    template<class PixelType>
    bool SaveGaussianPyramid(const GaussianPyramid<PixelType>& image, const std::string& path);

    // Concrete implementations for RGB8
    template<> extern const Image<RGB8> LoadImage(const std::string& path);
    template<> extern const ImageWithMask<RGB8> LoadImageWithMask(const std::string& path);
    template<> extern const Image<RGB8> LoadFromQImage(const QImage& image);
    template<> extern bool SaveImage(const Image<RGB8>& image, const std::string& path);
    template<> extern QImage SaveToQImage(const Image<RGB8>& image);
    template<> extern bool SaveImage(const ImageWithMask<RGB8>& image, const std::string& path);
    template<> extern bool SaveGaussianPyramid(const GaussianPyramid<RGB8>& pyramid, const std::string& path);

    extern const Image<Alpha8> LoadMaskFromQImage(const QImage& img);
}

#include "IO.inl"