#pragma once

#include "Image.h"
#include "Convert.h"
#include "RGB.h"
#include "Random.h"

namespace IRL
{
    class Offset
    {
    public:
        int16_t dx;
        int16_t dy;
    };

    class Properties
    {
    public:
    };

    template<class PixelType>
    class NearestNeighborField
    {
    public:
        NearestNeighborField(const Image<PixelType>& source,  int32_t targetWidth, int32_t targetHeight)
            : Source(source), OffsetField(targetWidth, targetHeight), PropertiesField(targetWidth, targetHeight)
        { }

        void RandomFill()
        {
            // TODO: make parallel?
            ASSERT(Source.IsValid());
            ASSERT(OffsetField.IsValid());

            Random rnd(0);

            const int32_t w = Source.Width();
            const int32_t h = Source.Height();

            // TODO: ranges?
            const int32_t rangeX = 50;
            const int32_t rangeY = 50;

            for (int32_t x = 0; x < OffsetField.Width(); x++)
            {
                for (int32_t y = 0; y < OffsetField.Height(); y++)
                {
                    int32_t dx = rnd.Uniform(-rangeX, rangeX);
                    int32_t dy = rnd.Uniform(-rangeY, rangeY);
                    if (x + dx < 0) dx = 0 - x;
                    if (x + dx > w) dx = w - x;
                    if (y + dy < 0) dy = 0 - y;
                    if (y + dy > h) dy = h - y;

                    OffsetField(x, y).dx = (uint16_t)dx;
                    OffsetField(x, y).dy = (uint16_t)dy;
                }
            }
        }

    public:
        Image<PixelType> Source;
        Image<Offset> OffsetField;
        Image<Properties> PropertiesField;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // enable conversion from Offset to RGB8 for visualization purposes
    template<> 
    void Convert(RGB8& to, const Offset& from);

    // enable accumulation for scaling
    template<class Coeff>
    class Accumulator<Offset, Coeff>
    {
    public:
        Accumulator()
        {
            x = 0;
            y = 0;
        }

        void Append(const Offset& pixel, Coeff c)
        {
            x += c * pixel.dx;
            y += c * pixel.dy;
        }

        const Offset GetSum(Coeff normalizer)
        {
            Offset result;
            result.dx = (uint16_t)(x / normalizer);
            result.dy = (uint16_t)(y / normalizer);
            return result;
        }

    private:
        int32_t x, y;
    };
}