#include "Includes.h"
#include "IO.h"
#include "Profiler.h"

#ifdef IRL_USE_QT
#include <QtGui/QImage>
#else
#error Implement ImageIO without Qt.
#endif

namespace IRL
{
    const Image<RGB8> LoadImage(const std::string& path)
    {
        Tools::Profiler profiler("LoadImage");
        QImage img(QString::fromStdString(path));
        if (img.isNull())
            return Image<RGB8>();
        Image<RGB8> result(img.width(), img.height());
        img.convertToFormat(QImage::Format_ARGB32);
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

    bool SaveImage(const Image<RGB8>& image, const std::string& path)
    {
        Tools::Profiler profiler("SaveImage");
        QImage img(image.Width(), image.Height(), QImage::Format_ARGB32);
        uint32_t* bits = (uint32_t*)img.bits();
        const RGB8* color = image.Data();
        const RGB8* end = color + image.Width() * image.Height();
        while (color != end)
        {
            *bits = color->ToRGB32();
            ++color;
            ++bits;
        }
        return img.save(QString::fromStdString(path));
    }

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