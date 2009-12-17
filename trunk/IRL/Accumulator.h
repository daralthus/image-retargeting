#pragma once

namespace IRL
{
    // Used for linear operation with pixels. 
    // Template specializations defined in files with color
    // definition (Lab.h, RGB.g).
    template<class PixelType, class Coeff>
    class Accumulator;
}