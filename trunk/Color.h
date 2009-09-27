#pragma once

#include <math.h>

#include <string>
#include <sstream>

#include "pstdint.h"
#include "Real.h"

namespace IRL
{
    // white point for D50
    const Real Xr = (Real)0.96420;
    const Real Yr = (Real)1.00000;
    const Real Zr = (Real)0.82491;

    const Real Min_L = 0;
    const Real Max_L = 100;
    const Real Min_a = (Real)-91.3727;
    const Real Max_a = (Real)96.7768;
    const Real Min_b = (Real)-125.845;
    const Real Max_b = (Real)81.7356;

    const Real LAB_delta  = (Real)(6.0 / 29.0);
    const Real LAB_InvDelta = (Real)(1.0 / LAB_delta);
    const Real LAB_delta3 = LAB_delta * LAB_delta * LAB_delta;

    class Color
    {
        template<class T>
        struct ValueType;
        template<class T>
        struct MaxValue;

        // uint8_t 
        template<>
        struct ValueType<uint8_t>
        {
            typedef uint32_t Type;
        };
        template<>
        struct MaxValue<uint8_t>
        {
            static const int Value = UINT8_MAX;
        };

        // uint16_t
        template<>
        struct ValueType<uint16_t>
        {
            typedef uint64_t Type;
        };
        template<>
        struct MaxValue<uint16_t>
        {
            static const int Value = UINT16_MAX;
        };

    public:
        typedef uint8_t Channel;

        enum Space
        {
            RGB = 0,
            Lab = 1
        };

        union
        {
            struct 
            {
                Channel L;
                Channel a;
                Channel b;
            };
            struct 
            {
                Channel B;
                Channel G;
                Channel R;
                Channel A;
            };

            ValueType<Channel>::Type Value;
        };

        Color()
        { }

        Color(const Color& c) : Value(c.Value)
        { }

        uint32_t ToARGB32() const
        {
            uint8_t Rb = R / (MaxValue<Channel>::Value / UINT8_MAX);
            uint8_t Gb = G / (MaxValue<Channel>::Value / UINT8_MAX);
            uint8_t Bb = B / (MaxValue<Channel>::Value / UINT8_MAX);
            uint8_t Ab = A / (MaxValue<Channel>::Value / UINT8_MAX);
            return (Ab << 24) | (Rb << 16) | (Gb << 8) | Bb;
        }

        static const Color FromARGB32(uint32_t color)
        {
            int Bb = (color & 0x000000ff) >> 0;
            int Gb = (color & 0x0000ff00) >> 8;
            int Rb = (color & 0x00ff0000) >> 16;
            int Ab = (color & 0xff000000) >> 24;

            Color result;
            result.R = Rb * (MaxValue<Channel>::Value / UINT8_MAX);
            result.G = Gb * (MaxValue<Channel>::Value / UINT8_MAX);
            result.B = Bb * (MaxValue<Channel>::Value / UINT8_MAX);
            result.A = Ab * (MaxValue<Channel>::Value / UINT8_MAX);
            return result;
        }

        static inline Color LabToRGB(const Color& lab)
        {
            // ranges must correspond to ones in FromRGB
            Real L = Denormalize(lab.L, Min_L, Max_L);
            Real a = Denormalize(lab.a, Min_a, Max_a); 
            Real b = Denormalize(lab.b, Min_b, Max_b);

            // L*a*b* -> XYZ

            // http://ru.wikipedia.org/wiki/LAB
            Real f_y = (L + 16) / 116;
            Real f_x = f_y + a / 500;
            Real f_z = f_y - b / 200;

            Real X = f_reverse(f_x) * Xr;
            Real Y = f_reverse(f_y) * Yr;
            Real Z = f_reverse(f_z) * Zr;

            // http://ru.wikipedia.org/wiki/RGB
            Real R =  ToReal(3.063) * X - ToReal(1.393) * Y - ToReal(0.476) * Z;
            Real G =  ToReal(-0.969)* X + ToReal(1.876) * Y + ToReal(0.042) * Z;
            Real B =  ToReal(0.068) * X - ToReal(0.229) * Y + ToReal(1.069) * Z;

            Color result;
            result.R = Normalize<Channel>(R, 0, 1);
            result.G = Normalize<Channel>(G, 0, 1);
            result.B = Normalize<Channel>(B, 0, 1);
            result.A = lab.A;

            return result;
        }

        static inline Color RGBToLab(const Color& rgb)
        {
            // RGB -> XYZ
            Real R = Denormalize(rgb.R, 0, 1);
            Real G = Denormalize(rgb.G, 0, 1);
            Real B = Denormalize(rgb.B, 0, 1);

            // http://ru.wikipedia.org/wiki/RGB
            Real X = ToReal(0.431) * R + ToReal(0.342) * G + ToReal(0.178) * B;
            Real Y = ToReal(0.222) * R + ToReal(0.707) * G + ToReal(0.071) * B;
            Real Z = ToReal(0.020) * R + ToReal(0.130) * G + ToReal(0.939) * B;

            // XYZ -> L*a*b*
            X = f(X / Xr);
            Y = f(Y / Yr);
            Z = f(Z / Zr);

            Real L = (116 * Y) - 16;
            Real a = 500 * (X - Y);
            Real b = 200 * (Y - Z);

            Color result;
            result.L = Normalize<Channel>(L, Min_L, Max_L);
            result.a = Normalize<Channel>(a, Min_a, Max_a); 
            result.b = Normalize<Channel>(b, Min_b, Max_b);
            result.A = rgb.A;

            return result;
        }

    private:
        static inline Real f(Real input)
        {
            // http://ru.wikipedia.org/wiki/LAB
            if (input > LAB_delta3)
                return RealPow(input, ToReal(1.0/3.0));
            else                    
                return (ToReal(1.0/3.0) * LAB_InvDelta * LAB_InvDelta * input) + ToReal(4.0 / 29.0);
        }

        static inline Real f_reverse(Real input)
        {
            // http://ru.wikipedia.org/wiki/LAB
            if (input > LAB_delta)
                return RealPow(input, 3);
            else                    
                return (input - ToReal(16.0 / 116.0)) * 3 * LAB_delta * LAB_delta;
        }

        template<class T>
        static inline T Normalize(Real val, Real min, Real max)
        {
            int result = (int)(MaxValue<T>::Value * (val - min) / (max - min));
            if (result < 0)
                result = 0;
            else
            {
                if (result > MaxValue<T>::Value)
                    result = MaxValue<T>::Value;
            }
            return (T)result;
        }

        template<class T>
        static inline Real Denormalize(T val, Real min, Real max)
        {
            return (Real)val * (max - min) / MaxValue<T>::Value + min;
        }
    };
}