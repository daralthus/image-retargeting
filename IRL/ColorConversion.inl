#include "ColorConversion.h"
#include "TypeTraits.h"

namespace IRL
{
    namespace Internal
    {
        // white point for D50
        const double Xr = 0.96420;
        const double Yr = 1.00000;
        const double Zr = 0.82491;

        const double Min_L = 0;
        const double Max_L = 100;
        const double Min_a = -91.3727;
        const double Max_a = 96.7768;
        const double Min_b = -125.845;
        const double Max_b = 81.7356;

        const double LAB_delta  = 6.0 / 29.0;
        const double LAB_InvDelta = 1.0 / LAB_delta;
        const double LAB_delta3 = LAB_delta * LAB_delta * LAB_delta;

        static inline double _f(double input)
        {
            // http://ru.wikipedia.org/wiki/LAB
            if (input > LAB_delta3)
                return pow(input, 1.0/3.0);
            else                    
                return (1.0 / 3.0 * LAB_InvDelta * LAB_InvDelta * input) + 4.0 / 29.0;
        }

        static inline double _f_reverse(double input)
        {
            // http://ru.wikipedia.org/wiki/LAB
            if (input > LAB_delta)
                return pow(input, 3);
            else                    
                return (input - 16.0 / 116.0) * 3 * LAB_delta * LAB_delta;
        }
    }

    // Lab -> RGB
    template<class RgbChannel, class LabChannel>
    void Convert(RGB<RgbChannel>& rgb, const Lab<LabChannel>& lab)
    {
        using namespace Internal;

        // ranges must correspond to ones in FromRGB
        double L = TypeTraits<LabChannel>::Denormalize(lab.L, Min_L, Max_L);
        double a = TypeTraits<LabChannel>::Denormalize(lab.a, Min_a, Max_a); 
        double b = TypeTraits<LabChannel>::Denormalize(lab.b, Min_b, Max_b);

        // L*a*b* -> XYZ

        // http://ru.wikipedia.org/wiki/LAB
        double f_y = (L + 16) / 116;
        double f_x = f_y + a / 500;
        double f_z = f_y - b / 200;

        double X = _f_reverse(f_x) * Xr;
        double Y = _f_reverse(f_y) * Yr;
        double Z = _f_reverse(f_z) * Zr;

        // http://ru.wikipedia.org/wiki/RGB
        double R =  3.063 * X - 1.393 * Y - 0.476 * Z;
        double G = -0.969 * X + 1.876 * Y + 0.042 * Z;
        double B =  0.068 * X - 0.229 * Y + 1.069 * Z;

        rgb.R = TypeTraits<RgbChannel>::Normalize(R, 0.0, 1.0);
        rgb.G = TypeTraits<RgbChannel>::Normalize(G, 0.0, 1.0);
        rgb.B = TypeTraits<RgbChannel>::Normalize(B, 0.0, 1.0);
    }

    // RGB -> Lab
    template<class LabChannel, class RgbChannel>
    void Convert(Lab<LabChannel>& lab, const RGB<RgbChannel>& rgb)
    {
        using namespace Internal;

        // RGB -> XYZ
        double R = TypeTraits<RgbChannel>::Denormalize(rgb.R, 0.0, 1.0);
        double G = TypeTraits<RgbChannel>::Denormalize(rgb.G, 0.0, 1.0);
        double B = TypeTraits<RgbChannel>::Denormalize(rgb.B, 0.0, 1.0);

        // http://ru.wikipedia.org/wiki/RGB
        double X = 0.431 * R + 0.342 * G + 0.178 * B;
        double Y = 0.222 * R + 0.707 * G + 0.071 * B;
        double Z = 0.020 * R + 0.130 * G + 0.939 * B;

        // XYZ -> L*a*b*
        X = _f(X / Xr);
        Y = _f(Y / Yr);
        Z = _f(Z / Zr);

        double L = (116 * Y) - 16;
        double a = 500 * (X - Y);
        double b = 200 * (Y - Z);

        ASSERT(L > Min_L - 0.1);
        ASSERT(L < Max_L + 0.1);

        lab.L = TypeTraits<LabChannel>::Normalize(L, Min_L, Max_L);
        lab.a = TypeTraits<LabChannel>::Normalize(a, Min_a, Max_a); 
        lab.b = TypeTraits<LabChannel>::Normalize(b, Min_b, Max_b);
    }

    // RGB <-> RGB
    template<class ChannelTo, class ChannelFrom>
    void Convert(RGB<ChannelTo>& to, const RGB<ChannelFrom>& from)
    {
        to.B = TypeTraits<ChannelTo>::Normalize(from.B, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
        to.G = TypeTraits<ChannelTo>::Normalize(from.G, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
        to.R = TypeTraits<ChannelTo>::Normalize(from.R, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
    }

    // Lab <-> Lab
    template<class ChannelTo, class ChannelFrom>
    void Convert(Lab<ChannelTo>& to, const Lab<ChannelFrom>& from)
    {
        to.L = TypeTraits<ChannelTo>::Normalize(from.L, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
        to.a = TypeTraits<ChannelTo>::Normalize(from.a, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
        to.b = TypeTraits<ChannelTo>::Normalize(from.b, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
    }

    template<class ChannelTo, class ChannelFrom>
    void Convert(Alpha<ChannelTo>& to, const Alpha<ChannelFrom>& from)
    {
        to.A = TypeTraits<ChannelTo>::Normalize(from.A, (ChannelFrom)0, TypeTraits<ChannelFrom>::MaxValue());
    }
}
