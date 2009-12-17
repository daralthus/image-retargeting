#pragma once

#include "Point2D.h"

namespace IRL
{
    template<class IntType>
    class Rectangle
    {
    public:
        Rectangle() {}

        Rectangle(IntType x, IntType y, IntType width, IntType height)
        {
            Left = x;
            Top = y;
            Right = x + width;
            Bottom = y + height;
        }

        template<class OtherIntType>
        force_inline bool Contains(const Point2D<OtherIntType>& p) const
        {
            return (p.x >= Left && p.x < Right) && (p.y >= Top && p.y < Bottom);
        }

        const IntType Area()
        {
            return (Right - Left) * (Bottom - Top);
        }

    public:
        IntType Left;
        IntType Right;
        IntType Top;
        IntType Bottom;
    };
}