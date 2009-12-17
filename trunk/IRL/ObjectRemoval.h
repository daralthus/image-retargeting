#pragma once

#include "Image.h"

namespace IRL
{
    template<class PixelType>
    const Image<PixelType> RemoveObject(const ImageWithMask<PixelType>& img);
}

#include "ObjectRemoval.inl"