#pragma once

#include "Image.h"
#include "Alpha.h"

namespace IRL
{
    template<class PixelType>
    class ImageWithMask
    {
    public:
        typedef Alpha<typename PixelType::ChannelType> AlphaChannel;
        IRL::Image<PixelType> Image;
        IRL::Image<AlphaChannel> Mask;

    public:
        ImageWithMask() {}
        ImageWithMask(const ImageWithMask& i) : Image(i.Image), Mask(i.Mask) {}
        ImageWithMask(const IRL::Image<PixelType>& img, const IRL::Image<AlphaChannel>& mask) : Image(img), Mask(mask) {}
        ImageWithMask& operator=(const ImageWithMask& i) { Image = i.Image; Mask = i.Mask; return *this; }
    };
}