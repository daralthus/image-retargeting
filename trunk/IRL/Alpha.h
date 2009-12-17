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

        force_inline bool IsMasked() const
        {
            return A < TypeTraits<Channel>::MaxValue() / 2;
        }

        static const Alpha FromRGB32(uint32_t color)
        {
            int Ab = (color & 0xff000000) >> 24;
            Channel A = TypeTraits<Channel>::Normalize(Ab, 0, UINT8_MAX);
            return Alpha(A);
        }

        static force_inline DistanceType Distance(const Alpha& a, const Alpha& b)
        {
            DistanceType DA = a.A - b.A;
            return DA * DA;
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
            Norm = 0;
        }

        force_inline void Append(const PixelType& pixel, Coeff c)
        {
            A += c * pixel.A;
        }

        force_inline const PixelType GetSum(Coeff normalizer) const
        {
            PixelType result;
            result.A = (ChannelType)(A / normalizer);
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
        LargerType A;
    };
}