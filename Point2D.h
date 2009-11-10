#pragma once

#include "Convert.h"
#include "TypeTraits.h"
#include "RGB.h"

namespace IRL
{
    template<class IntType>
    class Point2D
    {
    public:
        Point2D()
        { }

        template<class OtherIntType>
        Point2D(const Point2D<OtherIntType>& other)
        { 
            x = other.x;
            y = other.y;
        }

        Point2D(IntType x, IntType y)
        {
            this->x = x;
            this->y = y;
        }

        bool operator==(const Point2D& other) const
        {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Point2D& other) const
        {
            return x != other.x || y != other.y;
        }

        template<class OtherIntType>
        const Point2D<IntType> operator+(const Point2D<OtherIntType>& arg) const
        {
            return Point2D<IntType>(x + arg.x, y + arg.y);
        }
        template<class OtherIntType>
        const Point2D<IntType> operator-(const Point2D<OtherIntType>& arg) const
        {
            return Point2D<IntType>(x - arg.x, y - arg.y);
        }

    public:
        IntType x;
        IntType y;
    };

    typedef Point2D<int16_t> Point16;
    typedef Point2D<int32_t> Point32;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // enable conversion from Offset to RGB8 for visualization purposes
    template<> 
    void Convert(RGB8& to, const Point16& from);

    // enable accumulation for scaling
    template<class Coeff, class IntType>
    class Accumulator<Point2D<IntType>, Coeff>
    {
    public:
        Accumulator()
        {
            x = 0;
            y = 0;
        }

        void Append(const Point2D& pixel, Coeff c)
        {
            x += c * pixel.x;
            y += c * pixel.y;
        }

        const Point2D GetSum(Coeff normalizer)
        {
            Point2D result;
            result.x = (uint16_t)(x / normalizer);
            result.y = (uint16_t)(y / normalizer);
            return result;
        }

    private:
        int32_t x, y;
    };
}