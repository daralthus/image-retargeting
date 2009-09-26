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
    public:
        enum Space
        {
            RGB = 0,
            Lab = 1
        };

        union
        {
            struct 
            {
                uint16_t L;
                uint16_t a;
                uint16_t b;
            };
            struct 
            {
                uint8_t B;
                uint8_t G;
                uint8_t R;

                uint8_t Dummy0;
                uint16_t Dummy1;

                uint16_t A;
            };
            uint64_t Value;
        };

        Color()
        { }

        explicit Color(uint32_t argb) : Value(argb)
        { 
            A = (uint16_t)((argb & 0xff000000) >> 24) * (UINT16_MAX / UINT8_MAX);
        }

        Color(const Color& c) : Value(c.Value)
        { }

        static inline Color LabToRGB(const Color& lab)
        {
            // ranges must correspond to ones in FromRGB
            Real L = DenormalizeFrom16Bit(lab.L, Min_L, Max_L);
            Real a = DenormalizeFrom16Bit(lab.a, Min_a, Max_a); 
            Real b = DenormalizeFrom16Bit(lab.b, Min_b, Max_b);

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
            result.R = NormalizeTo8Bit(R);
            result.G = NormalizeTo8Bit(G);
            result.B = NormalizeTo8Bit(B);
            result.A = lab.A;

            return result;
        }

        static inline Color RGBToLab(const Color& rgb)
        {
            uint8_t Rb = rgb.R;
            uint8_t Gb = rgb.G;
            uint8_t Bb = rgb.B;

            // RGB -> XYZ
            Real R = (Rb / ToReal(255.0f));       //R from 0 to 255
            Real G = (Gb / ToReal(255.0f));       //G from 0 to 255
            Real B = (Bb / ToReal(255.0f));       //B from 0 to 255

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
            result.L = NormalizeTo16Bit(L, Min_L, Max_L);
            result.a = NormalizeTo16Bit(a, Min_a, Max_a); 
            result.b = NormalizeTo16Bit(b, Min_b, Max_b);
            result.A = rgb.A;

            return result;
        }

        uint32_t ToARGB() const
        {
            return ((A / (UINT16_MAX / UINT8_MAX)) << 24) | (R << 16) | (G << 8) | B;
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

        static inline uint16_t NormalizeTo16Bit(Real val, Real min, Real max)
        {
            Real res = (val - min) / (max - min);
            if (res < 0) 
                res = 0;
            else
            {
                if (res > ToReal(1.0f))
                    res = ToReal(1.0f);
            }
            return (uint16_t)(UINT16_MAX * res);
        }

        static inline Real DenormalizeFrom16Bit(uint16_t val, Real min, Real max)
        {
            return (Real)val * (max - min) / UINT16_MAX + min;
        }

        static inline uint8_t NormalizeTo8Bit(Real val)
        {
            int res = (int)(val * 255);
            if (res < 0)
                res = 0;
            else
            {
                if (res > 255)
                    res = 255;
            }
            return (uint8_t)res;
        }
    };
}