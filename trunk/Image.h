#pragma once

#include "Assert.h"
#include "Color.h"
#include "RefCounted.h"

#include <vector>

namespace IRL
{
    // Image is a two-dimensional array of pixels (Color's).
    // Supports copy on write.
    class Image
    {
    public:
        Image() : _ptr(NULL)
        {
        }

        Image(const Image& img) : _ptr(NULL)
        { 
            *this = img;
        }

        Image(uint32_t w, uint32_t h, Color::Space colorSpace)
        {
            _ptr = ImagePrivate::Create(w, h, colorSpace);
        }

        explicit Image(const std::string& path)
        {
            _ptr = ImagePrivate::Load(path);
        }

        ~Image()
        { 
            if (_ptr)
                _ptr->Release();
        }

        Image& operator=(const Image& img)
        {
            if (img._ptr == _ptr)
                return *this;
            if (img._ptr)
                img._ptr->Acquire();
            if (_ptr)
                _ptr->Release();
            _ptr = img._ptr;
            return *this;
        }

        bool IsValid() const 
        { 
            return _ptr != NULL;
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
        void MakePrivate();

        // Private shared data
        class ImagePrivate : 
            public RefCounted<ImagePrivate>
        {
            ImagePrivate(uint32_t w, uint32_t h, Color::Space colorSpace, Color* data);
        public:
            static ImagePrivate* Create(uint32_t w, uint32_t h, Color::Space colorSpace);
            static ImagePrivate* Load(const std::string& path);
            static void Delete(ImagePrivate* obj);
        
            ImagePrivate* Clone() const;
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

        ImagePrivate* _ptr;
    };
}