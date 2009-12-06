#pragma once

#include "Image.h"

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> ScaleDown(const Image<PixelType>& src);
 
    template<class PixelType>
    const Image<PixelType> ScaleUp(const Image<PixelType>& src);
}

#include "Scaling.inl"