#pragma once

#include "TypeTraits.h"
#include "Accumulator.h"

namespace IRL
{
    template<class Channel>
    class Alpha
    {
    public:
        typedef Channel ChannelType;
        typedef typename TypeTraits<Channel>::LargerType DistanceType;

        Channel A;

        Alpha()
        { }

        Alpha(Channel a)
        {
            A = a;
        }

        static const Alpha FromRGB32(uint32_t color)
        {
            int Ab = (color & 0xff000000) >> 24;
            Channel A = TypeTraits<Channel>::Normalize(Ab, 0, UINT8_MAX);
            return Alpha(A);
        }

        static DistanceType Distance(const Alpha& a, const Alpha& b)
        {
            return (a.A - b.A)*(a.A - b.A);
        }
    };

    typedef Alpha<uint8_t>  Alpha8;
    typedef Alpha<uint16_t> Alpha16;
    typedef Alpha<float>    AlphaFloat;
    typedef Alpha<double>   AlphaDouble;

    // Accumulator for Alpha types

    template<class ChannelType, class Coeff>
    class Accumulator<Alpha<ChannelType>, Coeff>
    {
    public:
        typedef Alpha<ChannelType> PixelType;
        typedef typename TypeTraits<ChannelType>::LargerType LargerType;

        Accumulator()
        {
            A = 0;
        }

        void Append(const PixelType& pixel, Coeff c)
        {
            A += c * pixel.A;
        }

        const PixelType GetSum(Coeff normalizer)
        {
            PixelType result;
            result.A = (ChannelType)(A / normalizer);
            return result;
        }

    private:
        LargerType A;
    };
}