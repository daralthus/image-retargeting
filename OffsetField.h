#pragma once

#include "Image.h"
#include "Point2D.h"
#include "Alpha.h"

namespace IRL
{
    typedef Image<Point16> OffsetField;

    extern OffsetField MakeRandomField(int width, int height, int sourceWidth, int sourceHeight);
    extern OffsetField MakeSmoothField(int width, int height, int sourceWidth, int sourceHeight);
    extern OffsetField& RemoveMaskedOffsets(OffsetField& field, const Image<Alpha8>& mask, int iterations = 40);
    extern OffsetField& ClampField(OffsetField& field, int sourceWidth, int sourceHeight);
    extern OffsetField& ShakeField(OffsetField& field, int shakeRadius, int sourceWidth, int sourceHeight);

    //////////////////////////////////////////////////////////////////////////
    // Helpers

    template<class PixelType>
    OffsetField MakeRandomField(const Image<PixelType>& target, const Image<PixelType>& source)
    {
        return MakeRandomField(target.Width(), target.Height(), source.Width(), source.Height());
    }

    template<class PixelType>
    OffsetField MakeSmoothField(const Image<PixelType>& target, const Image<PixelType>& source)
    {
        return MakeSmoothField(target.Width(), target.Height(), source.Width(), source.Height());
    }

    template<class PixelType>
    OffsetField& ClampField(OffsetField& field, const Image<PixelType>& source)
    {
        return ClampField(field, source.Width(), source.Height());
    }

    template<class PixelType>
    OffsetField& ShakeField(OffsetField& field, int radius, const Image<PixelType>& source)
    {
        return ShakeField(field, radius, source.Width(), source.Height());
    }
}