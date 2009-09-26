#pragma once

#include <math.h>
#include "Config.h"

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
}