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
            B = c3;
            G = c2;
            R = c1;
        }

        RGB(uint32_t color)
        {
            int Bb = (color & 0x000000ff) >> 0;
            int Gb = (color & 0x0000ff00) >> 8;
            int Rb = (color & 0x00ff0000) >> 16;

            B = TypeTraits<Channel>::Normalize(Bb, 0, UINT8_MAX);
            G = TypeTraits<Channel>::Normalize(Gb, 0, UINT8_MAX);
            R = TypeTraits<Channel>::Normalize(Rb, 0, UINT8_MAX);
        }

        uint32_t ToRGB32() const
        {
            uint8_t Bb = TypeTraits<Channel>::Denormalize(B, 0, UINT8_MAX);
            uint8_t Gb = TypeTraits<Channel>::Denormalize(G, 0, UINT8_MAX);
            uint8_t Rb = TypeTraits<Channel>::Denormalize(R, 0, UINT8_MAX);
            return (255 << 24) | (Rb << 16) | (Gb << 8) | Bb;
        }

        static const RGB FromRGB32(uint32_t color)
        {
            return RGB(color);
        }

        static force_inline DistanceType Distance(const RGB& a, const RGB& b)
        {
            DistanceType DB = a.B - b.B;
            DistanceType DG = a.G - b.G;
            DistanceType DR = a.R - b.R;
            return DB * DB + DG * DG + DR * DR;
        }

        static DistanceType DistanceUpperBound()
        {
            DistanceType maxValue = TypeTraits<Channel>::MaxValue();
            return 3 * maxValue * maxValue + 1; 
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
            Norm = 0;
        }

        force_inline void Append(const PixelType& pixel, Coeff c)
        {
            B += (LargerType)(c * pixel.B);
            G += (LargerType)(c * pixel.G);
            R += (LargerType)(c * pixel.R);
        }

        force_inline const PixelType GetSum(Coeff normalizer) const
        {
            PixelType result;
            result.B = (ChannelType)(B / normalizer);
            result.G = (ChannelType)(G / normalizer);
            result.R = (ChannelType)(R / normalizer);
            return result;
        }

        force_inline void AppendAndChangeNorm(const PixelType& pixel, Coeff c)
        {
            Append(pixel, c);
            Norm += c;
        }

        force_inline const PixelType GetSum() const
        {
            return GetSum(Norm);
        }

    public:
        Coeff Norm;

    private:
        LargerType B, G, R;
    };
}