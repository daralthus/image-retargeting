#include "Includes.h"
#include "OffsetField.h"
#include "Random.h"

namespace IRL
{
    OffsetField MakeRandomField(int width, int height, int sourceWidth, int sourceHeight)
    {
        OffsetField result(width, height);
        Random random;
        for (int32_t y = HalfPatchSize; y < height - HalfPatchSize; y++)
        {
            for (int32_t x = HalfPatchSize; x < width - HalfPatchSize; x++)
            {
                int32_t sx = random.Uniform<int32_t>(HalfPatchSize, sourceWidth - HalfPatchSize);
                int32_t sy = random.Uniform<int32_t>(HalfPatchSize, sourceHeight - HalfPatchSize);

                result(x, y).x = (uint16_t)(sx - x);
                result(x, y).y = (uint16_t)(sy - y);
            }
        }
        return result;
    }

    OffsetField MakeSmoothField(int width, int height, int sourceWidth, int sourceHeight)
    {
        OffsetField result(width, height);
        for (int32_t y = HalfPatchSize; y < height - HalfPatchSize; y++)
        {
            for (int32_t x = HalfPatchSize; x < width - HalfPatchSize; x++)
            {
                int32_t sx = x * sourceWidth  / width;
                int32_t sy = y * sourceHeight / height;

                result(x, y).x = (uint16_t)(sx - x);
                result(x, y).y = (uint16_t)(sy - y);
            }
        }
        return result;
    }

    OffsetField& RemoveMaskedOffsets(OffsetField& field, const Image<Alpha8>& mask, int iterations)
    {
        Random random;
        int left = HalfPatchSize;
        int right = mask.Width() - HalfPatchSize;
        int top = HalfPatchSize;
        int bottom = mask.Height() - HalfPatchSize;
        for (int32_t y = HalfPatchSize; y < field.Height() - HalfPatchSize; y++)
        {
            for (int32_t x = HalfPatchSize; x < field.Width() - HalfPatchSize; x++)
            {
                int32_t sx = field(x, y).x + x;
                int32_t sy = field(x, y).y + y;
                if (mask(sx, sy).IsMasked())
                {
                    int i = 0;
                    do 
                    {
                        sx = random.Uniform<int32_t>(left, right);
                        sy = random.Uniform<int32_t>(top,  bottom);
                        i++;
                    } while (mask(sx, sy).IsMasked() && i < iterations + 1);
                    field(x, y).x = (uint16_t)(sx - x);
                    field(x, y).y = (uint16_t)(sy - y);
                }
            }
        }
        return field;
    }
}