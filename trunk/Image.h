#pragma once

#include "Assert.h"
#include "Color.h"

#include <vector>

namespace IRL
{
    // Image is a two-dimensional array of pixels (Color's).
    // Acts as a shared pointer to private data.
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

        const Image Clone() const
        {
            if (IsValid())
            {
                Image result;
                result._ptr = _ptr->Clone();
                return result;
            } else
                return Image();
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
            return Data()[x + y * Width()];
        }

        const Color& Pixel(uint32_t x, uint32_t y) const
        {
            ASSERT(IsValid());
            ASSERT(x < Width());
            ASSERT(y < Height());
            return Data()[x + y * Width()];
        }

        void ChangeColorSpace(Color::Space newColorSpace)
        {
            ASSERT(IsValid());
            _ptr->ChangeColorSpace(newColorSpace);
        }
        
        bool Save(const std::string& path)
        {
            ASSERT(IsValid());
            return _ptr->Save(path);
        }

    private:
        // Private shared data
        class ImagePrivate
        {
            ImagePrivate(uint32_t w, uint32_t h, Color::Space colorSpace, 
                Color* data)
            {
                Width = w;
                Height = h;
                ColorSpace = colorSpace;
                Data = data;
                _refs = 1;
            }

            ~ImagePrivate()
            {
                ASSERT(_refs == 0);
            }

        public:
            static ImagePrivate* Create(uint32_t w, uint32_t h, Color::Space colorSpace);
            static ImagePrivate* Load(const std::string& path);

            uint32_t Width;
            uint32_t Height;
            Color::Space ColorSpace;
            Color* Data;

        public:
            void Acquire() const
            {
                _refs++; // TODO: make atomic
            }
            void Release() const
            {
                --_refs; // TODO: make atomic
                if (_refs == 0)
                    Delete(this);
            }

            ImagePrivate* Clone() const;

            void ChangeColorSpace(Color::Space newColorSpace)
            {
                if (newColorSpace == ColorSpace)
                    return;
                if (ColorSpace == Color::RGB && newColorSpace == Color::Lab)
                {
                    ConvertRGBToLab();
                    ColorSpace = newColorSpace;
                    return;
                }
                if (ColorSpace == Color::Lab && newColorSpace == Color::RGB)
                {
                    ConvertLabToRGB();
                    ColorSpace = newColorSpace;
                    return;
                }
                ASSERT(false && "No such color conversion defined");
            }

            void ConvertRGBToLab();
            void ConvertLabToRGB();

            bool Save(const std::string& path) const;

        private:
            mutable int32_t _refs;

            // Counterpart to Create and Load
            static void Delete(const ImagePrivate* ptr);
        };

        ImagePrivate* _ptr;
    };
}