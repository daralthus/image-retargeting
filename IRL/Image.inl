#include "Image.h"

namespace IRL
{
    template<class PixelType>
    void Image<PixelType>::MakePrivate()
    {
        ASSERT(IsValid());
        if (IsPrivate())
            return;
        Private* copy = _ptr->Clone();
        _ptr->Release();
        _ptr = copy;
    }

    template<class PixelType>
    Image<PixelType>& Image<PixelType>::operator=(const Image<PixelType>& obj)
    {
        if (obj._ptr == _ptr)
            return *this;
        if (obj._ptr)
            obj._ptr->Acquire();
        if (_ptr)
            _ptr->Release();
        _ptr = obj._ptr;
        return *this;
    }

    template<class PixelType>
    typename Image<PixelType>::Private* Image<PixelType>::Private::Create(int32_t w, int32_t h)
    {
        int sz = sizeof(Private) + w * h * sizeof(PixelType);
        uint8_t* ptr = (uint8_t*)malloc(sz);
        ASSERT(ptr != NULL);
        Private* res = (Private*)ptr;
        new(res) Private();
        res->Width = w;
        res->Height = h;
        res->Data = (PixelType*)(ptr + sizeof(Private));
        return res;
    }

    template<class PixelType>
    void Image<PixelType>::Private::Delete(typename Image<PixelType>::Private* obj)
    {
        free(obj);
    }

    template<class PixelType>
    typename Image<PixelType>::Private* Image<PixelType>::Private::Clone() const
    {
        Image::Private* res = Create(Width, Height);
        if (!res)
            return NULL;
        memcpy(res->Data, Data, sizeof(PixelType) * Width * Height);
        return res;
    }
}