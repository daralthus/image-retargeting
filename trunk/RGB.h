#pragma once

#include "TypeTraits.h"
#include "Accumulator.h"

namespace IRL
{
    template<class Channel>
    class RGB
    {
    public:
        typedef Channel ChannelType;
        typedef typename TypeTraits<Channel>::LargerType DistanceType;

        Channel B;
        Channel G;
        Channel R;

        RGB()
        { }

        RGB(Channel c1, Channel c2, Channel c3)
        {
            R = c1;
            G = c2;
            B = c3;
        }

        RGB(uint32_t color)
        {
            int Bb = (color & 0x000000ff) >> 0;
            int Gb = (color & 0x0000ff00) >> 8;
            int Rb = (color & 0x00ff0000) >> 16;

            R = TypeTraits<Channel>::Normalize(Rb, 0, UINT8_MAX);
            G = TypeTraits<Channel>::Normalize(Gb, 0, UINT8_MAX);
            B = TypeTraits<Channel>::Normalize(Bb, 0, UINT8_MAX);
        }

        uint32_t ToRGB32() const
        {
            uint8_t Rb = TypeTraits<Channel>::Denormalize(R, 0, UINT8_MAX);
            uint8_t Gb = TypeTraits<Channel>::Denormalize(G, 0, UINT8_MAX);
            uint8_t Bb = TypeTraits<Channel>::Denormalize(B, 0, UINT8_MAX);
            return (255 << 24) | (Rb << 16) | (Gb << 8) | Bb;
        }

        static const RGB FromRGB32(uint32_t color)
        {
            return RGB(color);
        }

        static DistanceType Distance(const RGB& a, const RGB& b)
        {
            return (a.R - b.R)*(a.R - b.R) + 
                   (a.G - b.G)*(a.G - b.G) + 
                   (a.B - b.B)*(a.B - b.B);
        }
    };

    typedef RGB<uint8_t>  RGB8;
    typedef RGB<uint16_t> RGB16;
    typedef RGB<float>    RGBFloat;
    typedef RGB<double>   RGBDouble;

    // Accumulator for RGB types

    template<class ChannelType, class Coeff>
    class Accumulator<RGB<ChannelType>, Coeff>
    {
    public:
        typedef RGB<ChannelType> PixelType;
        typedef typename TypeTraits<ChannelType>::LargerType LargerType;

        Accumulator()
        {
            B = 0;
            G = 0;
            R = 0;
        }

        void Append(const PixelType& pixel, Coeff c)
        {
            B += c * pixel.B;
            G += c * pixel.G;
            R += c * pixel.R;
        }

        const PixelType GetSum(Coeff normalizer)
        {
            PixelType result;
            result.B = (ChannelType)(B / normalizer);
            result.G = (ChannelType)(G / normalizer);
            result.R = (ChannelType)(R / normalizer);
            return result;
        }

    private:
        LargerType R, G, B;
    };
}