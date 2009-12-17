#include "GaussianPyramid.h"
#include "Scaling.h"
#include "Profiler.h"
#include "ImageConversion.h"

namespace IRL
{
    template<class PixelType>
    GaussianPyramid<PixelType>::GaussianPyramid()
    {
    }
    template<class PixelType>
    GaussianPyramid<PixelType>::GaussianPyramid(const Image<PixelType>& source, int levels)
    {
        Tools::Profiler profiler("GaussianPyramid::GaussianPyramid");
        ASSERT(levels > 0);
        Levels.resize(levels);
        Levels[0] = source;
        for (int i = 1; i < levels; i++)
            Levels[i] = ScaleDown(Levels[i - 1]);
    }

    // Converter for pyramid
    template<class ToPixelFormat, class FromPixelFormat>
    void Convert(GaussianPyramid<ToPixelFormat>& to, const GaussianPyramid<FromPixelFormat>& from)
    {
        to.Levels.resize(from.Levels.size());
        for (unsigned int i = 0; i < from.Levels.size(); i++)
            Convert(to.Levels[i], from.Levels[i]);
    }
}