#include "Includes.h"
#include "ObjectRemoval.h"

ObjectRemovalWorkItem::ObjectRemovalWorkItem(WorkingArea* workingArea, 
                                             const QImage& image, 
                                             const QPolygonF& mask, qreal scaleX, qreal scaleY) 
    : _image(image), _poly(mask), _scaleX(scaleX), _scaleY(scaleY), _workingArea(workingArea)
{
}

void ObjectRemovalWorkItem::execute()
{
    QImage surface(_image.size(), QImage::Format_Mono);
    prepareMask(surface);

    IRL::ImageWithMask<Color> imageWithMask;
    imageWithMask.Image = IRL::LoadFromQImage<Color>(_image);
    imageWithMask.Mask  = IRL::LoadMaskFromQImage(surface);

    _lastResultTime = clock();
    IRL::RemoveObject(imageWithMask, this);
}

void ObjectRemovalWorkItem::prepareMask(QImage& surface)
{
    QPainter painter(&surface);
    painter.fillRect(0, 0, surface.width(), surface.height(), QColor(255, 255, 255, 255));

    painter.setPen(Qt::NoPen);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::black);
    painter.setBrush(brush);

    painter.scale(_scaleX, _scaleY);
    painter.drawPolygon(_poly);
}

void ObjectRemovalWorkItem::IntermediateResult(const IRL::Image<Color>& result, int progress, int total)
{
    if (clock() - _lastResultTime > 50 * CLOCKS_PER_SEC / 1000)
    {
        pushUpdate(IRL::SaveToQImage(result), progress * 100 / total, false);
        _lastResultTime = clock();
    }
}

void ObjectRemovalWorkItem::OperationEnded(const IRL::Image<Color>& result)
{
    pushUpdate(IRL::SaveToQImage(result), 100, true);
}

void ObjectRemovalWorkItem::pushUpdate(const QImage& image, int progress, bool final)
{
    _workingArea->pushUpdate(image, _poly, progress, final);
}