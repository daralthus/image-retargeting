#pragma once

#include "CopyOnWrite.h"

namespace IRL
{
    // Image is a two-dimensional array of pixels.
    // Supports copy on write.

    template<class PixelType>
    class Image
    {
        IMPLEMENT_COPY_ON_WRITE(Image);
    public:
        Image(int32_t w, int32_t h)
        {
            _ptr = Private::Create(w, h);
        }

        int32_t Width() const { ASSERT(IsValid()); return _ptr->Width; }
        int32_t Height() const { ASSERT(IsValid()); return _ptr->Height; }

        PixelType* Data() { MakePrivate(); return &_ptr->Data[0]; }
        const PixelType* Data() const { ASSERT(IsValid()); return &_ptr->Data[0]; }

        PixelType& Pixel(int32_t x, int32_t y)
        {
            ASSERT(IsValid());
            ASSERT(x >= 0 && x < Width());
            ASSERT(y >= 0 && y < Height());
            MakePrivate();
            return _ptr->Data[x + y * Width()];
        }
        const PixelType& Pixel(int32_t x, int32_t y) const
        {
            ASSERT(IsValid());
            ASSERT(x >= 0 && x < Width());
            ASSERT(y >= 0 && y < Height());
            return _ptr->Data[x + y * Width()];
        }

        // more compact operator versions
        PixelType& operator()(int32_t x, int32_t y) { return Pixel(x, y); }
        const PixelType& operator()(int32_t x, int32_t y) const { return Pixel(x, y); }

    private:
        // Private shared data
        class Private : 
            public RefCounted<Private>
        {
        public:
            static Private* Create(int32_t w, int32_t h)
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
            static void Delete(Private* obj)
            {
                free(obj);
            }

            Private* Clone() const
            {
                Image::Private* res = Create(Width, Height);
                if (!res)
                    return NULL;
                memcpy(res->Data, Data, sizeof(PixelType) * Width * Height);
                return res;
            }

        public:
            int32_t Width;
            int32_t Height;
            PixelType* Data;
        };
    };
}