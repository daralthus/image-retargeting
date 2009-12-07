#pragma once

#include "TypeTraits.h"
#include "Accumulator.h"

namespace IRL
{
    template<class Channel>
    class Lab
    {
    public:
        typedef Channel ChannelType;
        typedef typename TypeTraits<Channel>::LargerType DistanceType;

        Channel L;
        Channel a;
        Channel b;

        Lab()
        { }

        Lab(Channel c1, Channel c2, Channel c3)
        {
            L = c1;
            a = c2;
            b = c3;
        }

        static force_inline DistanceType Distance(const Lab& a, const Lab& b)
        {
            return (a.L - b.L)*(a.L - b.L) + 
                   (a.a - b.a)*(a.a - b.a) + 
                   (a.b - b.b)*(a.b - b.b);
        }

        static DistanceType DistanceUpperBound()
        {
            return 3 * TypeTraits<Channel>::MaxValue() * TypeTraits<Channel>::MaxValue() + 1; 
        }
    };

    typedef Lab<uint8_t>  Lab8;
    typedef Lab<uint16_t> Lab16;
    typedef Lab<float>    LabFloat;
    typedef Lab<double>   LabDouble;

    // Accumulator for Lab types

    template<class ChannelType, class Coeff>
    class Accumulator<Lab<ChannelType>, Coeff>
    {
    public:
        typedef Lab<ChannelType> PixelType;
        typedef typename TypeTraits<ChannelType>::LargerType LargerType;

        Accumulator()
        {
            L = 0;
            a = 0;
            b = 0;
            Norm = 0;
        }

        force_inline void Append(const PixelType& pixel, Coeff c)
        {
            L += (LargerType)(c * pixel.L);
            a += (LargerType)(c * pixel.a);
            b += (LargerType)(c * pixel.b);
        }

        force_inline const PixelType GetSum(Coeff normalizer) const
        {
            PixelType result;
            result.L = (ChannelType)(L / normalizer);
            result.a = (ChannelType)(a / normalizer);
            result.b = (ChannelType)(b / normalizer);
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
        LargerType L, a, b;
    };
}