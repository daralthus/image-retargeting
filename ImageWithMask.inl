#include "ImageWithMask.h"
#include "Parallel.h"

namespace IRL
{
    template<class PixelType>
    Image<PixelType> MixImages(const Image<PixelType>& a, const Image<PixelType>& b, const Image<Alpha8>& mask)
    {
        Image<PixelType> target = a;
        for (int y = 0; y < a.Height(); y++)
        {
            for (int x = 0; x < a.Width(); x++)
            {
                if (mask(x, y).IsMasked())
                    target(x, y) = b(x, y);
            }
        }
        return target;
    }
}