#include "Includes.h"
#include "IO.h"
#include "Profiler.h"

namespace IRL
{
    template<> 
    const Image<RGB8> LoadFromQImage(const QImage& img)
    {
        Tools::Profiler profiler("LoadFromQImage");
        Image<RGB8> result(img.width(), img.height());
        img.convertToFormat(QImage::Format_RGB32);
        uint32_t* rgb = (uint32_t*)img.bits();
        RGB8* color = result.Data();
        RGB8* end = color + img.width() * img.height();
        while (color != end)
        {
            *color = RGB8::FromRGB32(*rgb);
            ++color;
            ++rgb;
        }
        return result;
    }

    const Image<Alpha8> LoadMaskFromQImage(const QImage& img)
    {
        Tools::Profiler profiler("LoadMaskFromQImage");
        Image<Alpha8> result(img.width(), img.height());
        img.convertToFormat(QImage::Format_Mono);
        Alpha8* color = result.Data();
        for (int y = 0; y < img.height(); y++)
        {
            for (int x = 0; x < img.width(); x++)
            {
                QRgb rgb = img.pixel(x, y);
                *color = qRed(rgb);
                ++color;
            }
        }
        return result;
    }

    template<>
    const Image<RGB8> LoadImage(const std::string& path)
    {
        Tools::Profiler profiler("LoadImage");
        QImage img(QString::fromStdString(path));
        if (img.isNull())
            return Image<RGB8>();
        return LoadFromQImage<RGB8>(img);
    }

    template<> 
    const ImageWithMask<RGB8> LoadImageWithMask(const std::string& path)
    {
        Tools::Profiler profiler("LoadImageWithMask");
        QImage img(QString::fromStdString(path));
        if (img.isNull())
            return ImageWithMask<RGB8>();
        Image<RGB8> result(img.width(), img.height());
        Image<Alpha8> mask(img.width(), img.height());
        img.convertToFormat(QImage::Format_ARGB32);
        uint32_t* rgb = (uint32_t*)img.bits();
        RGB8* color = result.Data();
        Alpha8* alpha = mask.Data();
        RGB8* end = color + img.width() * img.height();
        while (color != end)
        {
            *color = RGB8::FromRGB32(*rgb);
            *alpha = Alpha8::FromRGB32(*rgb);
            ++color;
            ++rgb;
            ++alpha;
        }
        return ImageWithMask<RGB8>(result, mask);
    }

    template<>
    bool SaveImage(const Image<RGB8>& image, const std::string& path)
    {
        Tools::Profiler profiler("SaveImage");
        return SaveToQImage<RGB8>(image).save(QString::fromStdString(path));
    }

    template<>
    QImage SaveToQImage(const Image<RGB8>& image)
    {
        Tools::Profiler profiler("SaveToQImage");
        QImage img(image.Width(), image.Height(), QImage::Format_RGB32);
        uint32_t* bits = (uint32_t*)img.bits();
        const RGB8* color = image.Data();
        const RGB8* end = color + image.Width() * image.Height();
        while (color != end)
        {
            *bits = color->ToRGB32();
            ++color;
            ++bits;
        }
        return img;
    }

    template<> 
    bool SaveImage(const ImageWithMask<RGB8>& image, const std::string& path)
    {
        ASSERT(image.Image.Width() == image.Mask.Width());
        ASSERT(image.Image.Height() == image.Mask.Height());

        Tools::Profiler profiler("SaveImage");
        QImage img(image.Image.Width(), image.Image.Height(), QImage::Format_ARGB32);
        QRgb* bits = (QRgb*)img.bits();
        const RGB8* color = image.Image.Data();
        const Alpha8* alpha = image.Mask.Data();
        const RGB8* end = color + image.Image.Width() * image.Image.Height();
        while (color != end)
        {
            QRgb rgb = color->ToRGB32();
            *bits = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), alpha->A);
            ++color;
            ++alpha;
            ++bits;
        }
        return img.save(QString::fromStdString(path));
    }

    template<>
    bool SaveGaussianPyramid(const GaussianPyramid<RGB8>& pyramid, const std::string& filePath)
    {
        std::string prefix;
        std::string ext;
        int dot = filePath.find_last_of('.');
        if (dot != -1)
        {
            prefix = filePath.substr(0, dot);
            ext = filePath.substr(dot, filePath.size() - dot);
        } else
            prefix = filePath;

        for (unsigned int i = 0; i < pyramid.Levels.size(); i++)
        {
            std::ostringstream fullPath;
            fullPath << prefix << "_" << i << ext;
            if (!SaveImage(pyramid.Levels[i], fullPath.str()))
                return false;
        }
        return true;
    }
}