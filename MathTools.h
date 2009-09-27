#pragma once

#include "pstdint.h"

namespace IRL
{
    // http://www.codecodex.com/wiki/Calculate_an_integer_square_root#C
    inline unsigned long isqrt(unsigned long x)
    {
        register unsigned long op, res, one;

        op = x;
        res = 0;

        /* "one" starts at the highest power of four <= than the argument. */
        one = 1 << 30;  /* second-to-top bit set */
        while (one > op) one >>= 2;

        while (one != 0) {
            if (op >= res + one) {
                op = op - (res + one);
                res = res +  2 * one;
            }
            res >>= 1;
            one >>= 2;
        }
        return(res);
    }
}