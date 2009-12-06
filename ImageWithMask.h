#pragma once

#include "Image.h"
#include "Alpha.h"

namespace IRL
{
    template<class PixelType>
    class ImageWithMask
    {
    public:
        IRL::Image<PixelType> Image;
        IRL::Image<Alpha8> Mask;

    public:
        ImageWithMask() {}
        ImageWithMask(const ImageWithMask& i) : Image(i.Image), Mask(i.Mask) {}
        ImageWithMask(const IRL::Image<PixelType>& img, const IRL::Image<Alpha8>& mask) : Image(img), Mask(mask) {}
        ImageWithMask& operator=(const ImageWithMask& i) { Image = i.Image; Mask = i.Mask; return *this; }
    };
}