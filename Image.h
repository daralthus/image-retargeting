#pragma once

#include "Color.h"
#include "CopyOnWrite.h"

namespace IRL
{
    // Image is a two-dimensional array of pixels (Color's).
    // Supports copy on write.
    class Image
    {
        IMPLEMENT_COPY_ON_WRITE(Image);
    public:
        Image(int32_t w, int32_t h, Color::Space colorSpace)
        {
            _ptr = Private::Create(w, h, colorSpace);
        }

        explicit Image(const std::string& path)
        {
            _ptr = Private::Load(path);
        }

        int32_t Width() const
        {
            ASSERT(IsValid());
            return _ptr->Width;
        }

        int32_t Height() const
        {
            ASSERT(IsValid());
            return _ptr->Height;
        }

        Color::Space ColorSpace() const
        {
            ASSERT(IsValid());
            return _ptr->ColorSpace;
        }

        Color* Data() 
        {
            ASSERT(IsValid());
            MakePrivate();
            return &_ptr->Data[0];
        }

        const Color* Data() const
        {
            ASSERT(IsValid());
            return &_ptr->Data[0];
        }

        Color& Pixel(int32_t x, int32_t y)
        {
            ASSERT(IsValid());
            ASSERT(x < Width());
            ASSERT(y < Height());
            MakePrivate();
            return _ptr->Data[x + y * Width()];
        }

        const Color& Pixel(int32_t x, int32_t y) const
        {
            ASSERT(IsValid());
            ASSERT(x >= 0 && x < Width());
            ASSERT(y < Height());
            return _ptr->Data[x + y * Width()];
        }

        void ChangeColorSpace(Color::Space newColorSpace)
        {
            ASSERT(IsValid());
            MakePrivate();
            _ptr->ChangeColorSpace(newColorSpace);
        }
        
        bool Save(const std::string& path) const
        {
            ASSERT(IsValid());
            return _ptr->Save(path);
        }

    private:
        // Private shared data
        class Private : 
            public RefCounted<Private>
        {
            Private(int32_t w, int32_t h, Color::Space colorSpace, Color* data);
        public:
            static Private* Create(int32_t w, int32_t h, Color::Space colorSpace);
            static Private* Load(const std::string& path);
            static void Delete(Private* obj);
        
            Private* Clone() const;
            bool Save(const std::string& path) const;

            void ChangeColorSpace(Color::Space newColorSpace);
            void ConvertRGBToLab();
            void ConvertLabToRGB();

        public:
            int32_t Width;
            int32_t Height;
            Color::Space ColorSpace;
            Color* Data;
        };
    };
}