#pragma once

#include "Convert.h"
#include "Image.h"

namespace IRL
{
    template<class ToPixelType, class FromPixelType>
    void Convert(Image<ToPixelType>& to, const Image<FromPixelType>& from);
}

#include "ImageConversion.inl"