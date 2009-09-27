#include "Image.h"
#include "Config.h"
#include "Profiler.h"

#ifdef IRL_USE_QT
#include <QtGui/QImage>
#endif

namespace IRL
{
    //////////////////////////////////////////////////////////////////////////
    // Create and Free 

    Image::ImagePrivate* Image::ImagePrivate::Create(uint32_t w, uint32_t h, Color::Space colorSpace)
    {
        int sz = sizeof(Image::ImagePrivate) + w * h * sizeof(Color);
        uint8_t* ptr = (uint8_t*)malloc(sz);
        ASSERT(ptr != NULL);
        Image::ImagePrivate* res = (Image::ImagePrivate*)ptr;
        new(res) Image::ImagePrivate(w, h, colorSpace, (Color*)(ptr + sizeof(Image::ImagePrivate)));
        return res;
    }

    void Image::ImagePrivate::Delete(const ImagePrivate* ptr)
    {
        ptr->~ImagePrivate();
        free((void*)ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    // Template conversion routine.

    template<class ConvertProcType>
    void ConvertColorSpace(Color* ptr, uint32_t w, uint32_t h, ConvertProcType convert)
    {
        // TODO: make parallel

        Color* end = ptr + w * h;
        while (ptr < end)
        {
            *ptr = convert(*ptr);
            ++ptr;
        }
    }

    void Image::ImagePrivate::ConvertRGBToLab()
    {
        ASSERT(ColorSpace == Color::RGB);
        Tools::Profiler profiler("ImagePrivate::ConvertRGBToLab");
        ConvertColorSpace(Data, Width, Height, Color::RGBToLab);
    }
 
    void Image::ImagePrivate::ConvertLabToRGB()
    {
        ASSERT(ColorSpace == Color::Lab);
        Tools::Profiler profiler("ImagePrivate::ConvertLabToRGB");
        ConvertColorSpace(Data, Width, Height, Color::LabToRGB);
    }

    //////////////////////////////////////////////////////////////////////////
    // Cloning

    Image::ImagePrivate* Image::ImagePrivate::Clone() const
    {
        Tools::Profiler profiler("ImagePrivate::Clone");
        Image::ImagePrivate* res = Create(Width, Height, ColorSpace);
        if (!res)
            return NULL;
        memcpy(res->Data, Data, sizeof(Color) * Width * Height);
        return res;
    }

    //////////////////////////////////////////////////////////////////////////
    // Image loading & saving

#ifdef IRL_USE_QT
    Image::ImagePrivate* Image::ImagePrivate::Load(const std::string& path)
    {
        Tools::Profiler profiler("ImagePrivate::Load");
        QImage img(QString::fromStdString(path));
        if (img.isNull())
            return NULL;
        Image::ImagePrivate* result = Create(img.width(), img.height(), Color::RGB);
        img.convertToFormat(QImage::Format_ARGB32);
        uint32_t* rgb = (uint32_t*)img.bits();
        Color* color = result->Data;
        Color* end = color + img.width() * img.height();
        while (color != end)
        {
            *color = Color::FromARGB32(*rgb);
            ++color;
            ++rgb;
        }
        return result;
    }

    bool Image::ImagePrivate::Save(const std::string& path) const
    {
        Tools::Profiler profiler("ImagePrivate::Save");
        ImagePrivate* src = (ImagePrivate*)this;

        // Convert to RGBA if required
        if (ColorSpace != Color::RGB)
        {
            src = Clone();
            if (!src)
                return false;
            src->ChangeColorSpace(Color::RGB);
        } else
            src->Acquire();

        QImage img(Width, Height, QImage::Format_ARGB32);
        uint32_t* bits = (uint32_t*)img.bits();
        Color* color = src->Data;
        Color* end = color + Width * Height;
        while (color != end)
        {
            *bits = color->ToARGB32();
            ++color;
            ++bits;
        }

        bool result = img.save(QString::fromStdString(path));
        src->Release();
        return result;
    }
#else
#error Implement Image::ImagePrivate::Load and Save
#endif
}