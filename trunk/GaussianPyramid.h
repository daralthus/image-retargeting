#pragma once

#include "Image.h"
#include "Convert.h"

namespace IRL
{
    template<class PixelType>
    class GaussianPyramid
    {
    public:
        GaussianPyramid();
        GaussianPyramid(const Image<PixelType>& source, int levels);

    public:
        std::vector<Image<PixelType> > Levels;
    };

    // Converter for pyramid
    template<class ToPixelFormat, class FromPixelFormat>
    void Convert(GaussianPyramid<ToPixelFormat>& to, const GaussianPyramid<FromPixelFormat>& from);
}

#include "GaussianPyramid.inl"