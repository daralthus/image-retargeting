#include "Includes.h"
#include "OffsetField.h"
#include "Random.h"
#include "Profiler.h"

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
        if (!mask.IsValid())
            return field;
    
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
                    int32_t nsx = sx;
                    int32_t nsy = sy;
                    int32_t r = 2;
                    do 
                    {
                        nsx += random.Uniform<int32_t>(-r, r);
                        nsy += random.Uniform<int32_t>(-r, r);
                        if (nsx < left) nsx = left;
                        if (nsx >= right) nsx = right - 1;
                        if (nsy < top) nsy = top;
                        if (nsy >= bottom) nsy = bottom - 1;
                        i++;
                        if ((i % 2) == 0)
                            r *= 2;
                    } while (mask(nsx, nsy).IsMasked() && i < iterations + 1);
                    field(x, y).x = (uint16_t)(nsx - x);
                    field(x, y).y = (uint16_t)(nsy - y);
                }
            }
        }
        return field;
    }

    OffsetField& ClampField(OffsetField& field, int sourceWidth, int sourceHeight)
    {
        for (int y = HalfPatchSize; y < field.Height() - HalfPatchSize; y++)
        {
            for (int x = HalfPatchSize; x < field.Width() - HalfPatchSize; x++)
            {
                int sx = x + field(x, y).x;
                int sy = y + field(x, y).y;
                if (sx < HalfPatchSize) sx = HalfPatchSize;
                if (sx >= sourceWidth - HalfPatchSize) sx = sourceWidth - HalfPatchSize - 1;
                if (sy < HalfPatchSize) sy = HalfPatchSize;
                if (sy >= sourceHeight - HalfPatchSize) sy = sourceHeight - HalfPatchSize - 1;
                field(x, y).x = sx - x;
                field(x, y).y = sy - y;
            }
        }
        return field;
    }

    extern OffsetField& ShakeField(OffsetField& field, int shakeRadius, int sourceWidth, int sourceHeight)
    {
        Random random;
        for (int y = HalfPatchSize; y < field.Height() - HalfPatchSize; y++)
        {
            for (int x = HalfPatchSize; x < field.Width() - HalfPatchSize; x++)
            {
                int sx = x + field(x, y).x + random.Uniform<int>(-shakeRadius, +shakeRadius);
                int sy = y + field(x, y).y + random.Uniform<int>(-shakeRadius, +shakeRadius);
                if (sx < HalfPatchSize) sx = HalfPatchSize;
                if (sx >= sourceWidth - HalfPatchSize) sx = sourceWidth - HalfPatchSize - 1;
                if (sy < HalfPatchSize) sy = HalfPatchSize;
                if (sy >= sourceHeight - HalfPatchSize) sy = sourceHeight - HalfPatchSize - 1;
                field(x, y).x = sx - x;
                field(x, y).y = sy - y;
            }
        }
        return field;
    }
}