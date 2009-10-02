#pragma once

#include <math.h>

#include <string>
#include <sstream>

#include "pstdint.h"
#include "Real.h"
#include "MathTools.h"

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
        struct Traits;

        // uint8_t 
        template<>
        struct Traits<uint8_t>
        {
            static const int MaxValue = UINT8_MAX;
        };

        // uint16_t
        template<>
        struct Traits<uint16_t>
        {
            static const int MaxValue = UINT16_MAX;
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
        };

        Color()
        { }

        Color(Channel c1, Channel c2, Channel c3, Channel c4)
        {
            R = c1;
            G = c2;
            B = c3;
            A = c4;
        }

        uint32_t ToARGB32() const
        {
            uint8_t Rb = R / (Traits<Channel>::MaxValue / UINT8_MAX);
            uint8_t Gb = G / (Traits<Channel>::MaxValue / UINT8_MAX);
            uint8_t Bb = B / (Traits<Channel>::MaxValue / UINT8_MAX);
            uint8_t Ab = A / (Traits<Channel>::MaxValue / UINT8_MAX);
            return (Ab << 24) | (Rb << 16) | (Gb << 8) | Bb;
        }

        static const Color FromARGB32(uint32_t color)
        {
            int Bb = (color & 0x000000ff) >> 0;
            int Gb = (color & 0x0000ff00) >> 8;
            int Rb = (color & 0x00ff0000) >> 16;
            int Ab = (color & 0xff000000) >> 24;

            Color result;
            result.R = Rb * (Traits<Channel>::MaxValue / UINT8_MAX);
            result.G = Gb * (Traits<Channel>::MaxValue / UINT8_MAX);
            result.B = Bb * (Traits<Channel>::MaxValue / UINT8_MAX);
            result.A = Ab * (Traits<Channel>::MaxValue / UINT8_MAX);
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

        static uint32_t Distance(const Color& c1, const Color& c2)
        {
            const int MaxValueSquare  = Traits<Channel>::MaxValue * Traits<Channel>::MaxValue;

            const int DistanceLFactor = (int)(MaxValueSquare / ((Max_L - Min_L) * (Max_L - Min_L)));
            const int DistanceAFactor = (int)(MaxValueSquare / ((Max_a - Min_a) * (Max_a - Min_a)));
            const int DistanceBFactor = (int)(MaxValueSquare / ((Max_b - Min_b) * (Max_b - Min_b)));

            int32_t dL = c1.L - c2.L;
            dL *= dL;
            dL /= DistanceLFactor;
            int32_t da = c1.a - c2.a;
            da *= da;
            da /= DistanceAFactor;
            int32_t db = c1.b - c2.b;
            db *= db;
            db /= DistanceBFactor;

            return isqrt(dL + da + db);
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
            int result = (int)(Traits<T>::MaxValue * (val - min) / (max - min));
            if (result < 0)
                result = 0;
            else
            {
                if (result > Traits<T>::MaxValue)
                    result = Traits<T>::MaxValue;
            }
            return (T)result;
        }

        template<class T>
        static inline Real Denormalize(T val, Real min, Real max)
        {
            return (Real)val * (max - min) / Traits<T>::MaxValue + min;
        }
    };
}