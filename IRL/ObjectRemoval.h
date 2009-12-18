#pragma once

#include "Image.h"

namespace IRL
{
    template<class PixelType>
    class OperationCallback
    {
    public:
        virtual void IntermediateResult(const Image<PixelType>& result, int progress, int total) {(void)result; (void)progress; (void)total;}
        virtual void OperationEnded(const Image<PixelType>&) {}
    };

    template<class PixelType>
    const Image<PixelType> RemoveObject(const ImageWithMask<PixelType>& img, OperationCallback<PixelType>* callback = NULL);
}

#include "ObjectRemoval.inl"