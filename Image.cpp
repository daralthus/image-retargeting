#include "Includes.h"
#include "Image.h"
#include "Profiler.h"
#include "Parallel.h"

#ifdef IRL_USE_QT
#include <QtGui/QImage>
#endif

namespace IRL
{
    //////////////////////////////////////////////////////////////////////////
    // Create, Free, Copy on write

    Image::Private* Image::Private::Create(uint32_t w, uint32_t h, Color::Space colorSpace)
    {
        int sz = sizeof(Image::Private) + w * h * sizeof(Color);
        uint8_t* ptr = (uint8_t*)malloc(sz);
        ASSERT(ptr != NULL);
        Image::Private* res = (Image::Private*)ptr;
        new(res) Image::Private(w, h, colorSpace, (Color*)(ptr + sizeof(Image::Private)));
        return res;
    }

    void Image::Private::Delete(Private* obj)
    {
        obj->~Private();
        free(obj);
    }

    Image::Private::Private(uint32_t w, uint32_t h, Color::Space colorSpace, 
        Color* data)
    {
        Width = w;
        Height = h;
        ColorSpace = colorSpace;
        Data = data;
    }

    //////////////////////////////////////////////////////////////////////////
    // Template conversion routine.

    template<class Proc>
    class ConvertTask :
        public Parallel::Runnable
    {
        Color* _start;
        Color* _end;
        Proc _conv;
    public:
        void Set(Color* start, Color* end, Proc conv)
        {
            _start = start;
            _end = end;
            _conv = conv;
        }

        virtual void Run()
        {
            Color* ptr = _start;
            while (ptr < _end)
            {
                *ptr = _conv(*ptr);
                ++ptr;
            }
        }
    };

    template<class ConvertProcType>
    void ConvertColorSpace(Color* ptr, uint32_t w, uint32_t h, ConvertProcType convert)
    {
        Parallel::ParallelFor<ConvertTask<ConvertProcType>, 
            ConvertProcType> tasks(ptr, ptr + w * h, convert);
        tasks.SpawnAndSync();
    }

    void Image::Private::ChangeColorSpace(Color::Space newColorSpace)
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

    void Image::Private::ConvertRGBToLab()
    {
        ASSERT(ColorSpace == Color::RGB);
        Tools::Profiler profiler("Image::Private::ConvertRGBToLab");
        ConvertColorSpace(Data, Width, Height, Color::RGBToLab);
    }
 
    void Image::Private::ConvertLabToRGB()
    {
        ASSERT(ColorSpace == Color::Lab);
        Tools::Profiler profiler("Image::Private::ConvertLabToRGB");
        ConvertColorSpace(Data, Width, Height, Color::LabToRGB);
    }

    //////////////////////////////////////////////////////////////////////////
    // Cloning

    Image::Private* Image::Private::Clone() const
    {
        Image::Private* res = Create(Width, Height, ColorSpace);
        if (!res)
            return NULL;
        memcpy(res->Data, Data, sizeof(Color) * Width * Height);
        return res;
    }

    //////////////////////////////////////////////////////////////////////////
    // Image loading & saving

#ifdef IRL_USE_QT
    Image::Private* Image::Private::Load(const std::string& path)
    {
        Tools::Profiler profiler("Image::Private::Load");
        QImage img(QString::fromStdString(path));
        if (img.isNull())
            return NULL;
        Image::Private* result = Create(img.width(), img.height(), Color::RGB);
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

    bool Image::Private::Save(const std::string& path) const
    {
        Tools::Profiler profiler("Image::Private::Save");
        Private* src = (Private*)this;

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
#error Implement Image::Private::Load and Save
#endif
}