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
            DistanceType DL = a.L - b.L;
            DistanceType Da = a.a - b.a;
            DistanceType Db = a.b - b.b;
            return DL * DL * Multiplier<Channel>::L() * Multiplier<Channel>::L() + 
                   Da * Da * Multiplier<Channel>::a() * Multiplier<Channel>::a() +
                   Db * Db * Multiplier<Channel>::b() * Multiplier<Channel>::b();
        }

        static DistanceType DistanceUpperBound()
        {
            DistanceType maxValue = TypeTraits<Channel>::MaxValue();
            return 3 * maxValue * maxValue + 1; 
        }

    private:
        template<class PixelType>
        struct Multiplier;

        template<>
        struct  Multiplier<double>
        {
            static double L() { return 100.0; }               // L \in [0, 100]
            static double a() { return 96.7768 + 91.3727; }   // a \in [-91.3727, 96.7768]
            static double b() { return 81.7356 + 125.845; }  // b \in [-125.845, 81.7356]
        };

        template<>
        struct  Multiplier<float>
        {
            static float L() { return 100.0f; }               // L \in [0, 100]
            static float a() { return 96.7768f + 91.3727f; }   // a \in [-91.3727, 96.7768]
            static float b() { return 81.7356f + 125.845f; }  // b \in [-125.845, 81.7356]
        };
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