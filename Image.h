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

        inline bool IsValid() const {  return _ptr != NULL; }
        inline bool IsPrivate() const { ASSERT(IsValid()); return _ptr->GetRefs() == 1; }

        inline int32_t Width() const { ASSERT(IsValid()); return _ptr->Width; }
        inline int32_t Height() const { ASSERT(IsValid()); return _ptr->Height; }

        inline PixelType* Data() { MakePrivate(); return &_ptr->Data[0]; }
        inline const PixelType* Data() const { ASSERT(IsValid()); return &_ptr->Data[0]; }

        inline void Clear() { memset(Data(), 0, sizeof(PixelType) * Width() * Height()); }

        force_inline PixelType& Pixel(int32_t x, int32_t y)
        {
            ASSERT(IsValid());
            ASSERT(x >= 0 && x < Width());
            ASSERT(y >= 0 && y < Height());
            MakePrivate();
            return _ptr->Data[x + y * Width()];
        }

        force_inline const PixelType& Pixel(int32_t x, int32_t y) const
        {
            ASSERT(IsValid());
            ASSERT(x >= 0 && x < Width());
            ASSERT(y >= 0 && y < Height());
            return _ptr->Data[x + y * Width()];
        }

        // more compact operator versions
        force_inline PixelType& operator()(int32_t x, int32_t y) { return Pixel(x, y); }
        force_inline const PixelType& operator()(int32_t x, int32_t y) const { return Pixel(x, y); }

    private:
        inline void MakePrivate();

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