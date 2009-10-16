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
        Image(uint32_t w, uint32_t h, Color::Space colorSpace)
        {
            _ptr = Private::Create(w, h, colorSpace);
        }

        explicit Image(const std::string& path)
        {
            _ptr = Private::Load(path);
        }

        uint32_t Width() const
        {
            ASSERT(IsValid());
            return _ptr->Width;
        }

        uint32_t Height() const
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

        Color& Pixel(uint32_t x, uint32_t y)
        {
            ASSERT(IsValid());
            ASSERT(x < Width());
            ASSERT(y < Height());
            MakePrivate();
            return _ptr->Data[x + y * Width()];
        }

        const Color& Pixel(uint32_t x, uint32_t y) const
        {
            ASSERT(IsValid());
            ASSERT(x < Width());
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
            Private(uint32_t w, uint32_t h, Color::Space colorSpace, Color* data);
        public:
            static Private* Create(uint32_t w, uint32_t h, Color::Space colorSpace);
            static Private* Load(const std::string& path);
            static void Delete(Private* obj);
        
            Private* Clone() const;
            bool Save(const std::string& path) const;

            void ChangeColorSpace(Color::Space newColorSpace);
            void ConvertRGBToLab();
            void ConvertLabToRGB();

        public:
            uint32_t Width;
            uint32_t Height;
            Color::Space ColorSpace;
            Color* Data;
        };
    };
}