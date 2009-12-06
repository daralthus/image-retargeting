#pragma once

#include "RefCounted.h"

namespace IRL
{
    template<class PixelType>
    class Image
    {
    public:
        Image() : _ptr(NULL) {}
        Image(int32_t w, int32_t h) { _ptr = Private::Create(w, h); }
        Image(const Image& obj) : _ptr(NULL) {  *this = obj; }
        ~Image() {  if (_ptr) _ptr->Release(); }
        Image& operator=(const Image& obj);

        bool IsValid() const {  return _ptr != NULL; }
        bool IsPrivate() const { ASSERT(IsValid()); return _ptr->GetRefs() == 1; }

        int32_t Width() const { ASSERT(IsValid()); return _ptr->Width; }
        int32_t Height() const { ASSERT(IsValid()); return _ptr->Height; }

        PixelType* Data() { MakePrivate(); return &_ptr->Data[0]; }
        const PixelType* Data() const { ASSERT(IsValid()); return &_ptr->Data[0]; }

        bool IsValidPixelCoordinates(int32_t x, int32_t y) const 
        {
            return ((x >= 0 && x < Width()) && (y >= 0 && y < Height()));
        }

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

        const PixelType& PixelWithMirroring(int32_t x, int32_t y) const;
        template<bool Mirroring>
        const PixelType& GetPixel(int32_t x, int32_t y) const { return Pixel(x, y); }
        template<>
        const PixelType& GetPixel<true>(int32_t x, int32_t y) const { return PixelWithMirroring(x, y); }

    private:
        void MakePrivate();

        // Private shared data
        class Private : 
            public RefCounted<Private>
        {
        public:
            static Private* Create(int32_t w, int32_t h);
            static void Delete(Private* obj);
            Private* Clone() const;
        public:
            int32_t Width;
            int32_t Height;
            PixelType* Data;
        };

        Private* _ptr;
    };
}

#include "Image.inl"