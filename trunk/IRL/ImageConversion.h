#pragma once

#include "Convert.h"
#include "Image.h"
#include "ImageWithMask.h"

namespace IRL
{
    template<class ToPixelType, class FromPixelType>
    void Convert(Image<ToPixelType>& to, const Image<FromPixelType>& from);

    template<class ToPixelType, class FromPixelType>
    void Convert(ImageWithMask<ToPixelType>& to, const ImageWithMask<FromPixelType>& from);
}

#include "ImageConversion.inl"