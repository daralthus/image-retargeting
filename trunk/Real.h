#pragma once

namespace IRL
{
#ifdef IRL_DOUBLE_PRECISION
    typedef double Real;
#else
    typedef float Real;
#endif

    template<class T>
    Real ToReal(const T val)
    {
        return (Real)val;
    }

    inline Real RealPow(Real x, Real y)
    {
#ifdef IRL_DOUBLE_PRECISION
        return pow(x, y);
#else
        return powf(x, y);
#endif
    }

    inline Real RealSqrt(Real x)
    {
#ifdef IRL_DOUBLE_PRECISION
        return sqrt(x);
#else
        return sqrtf(x);
#endif
    }
}