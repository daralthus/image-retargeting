#include "Includes.h"
#include "WorkingArea.h"
#include "MainWindow.h"

class WorkingAreaItem : public QGraphicsPixmapItem
{
public:
    WorkingAreaItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap)
    { 
        _backgroundBrush.setTexture(QPixmap(":/images/checker.png"));
    }

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QTransform t = painter->transform();
        qreal scaleX = sqrt(t.m11()*t.m11() + t.m12()*t.m12());
        qreal scaleY = sqrt(t.m21()*t.m21() + t.m22()*t.m22());
        QTransform brushTransform;
        brushTransform.scale(1.0 / scaleX, 1.0 / scaleY);
        _backgroundBrush.setTransform(brushTransform);

        painter->fillRect(option->exposedRect, _backgroundBrush);
        QGraphicsPixmapItem::paint(painter, option, widget);
    }

private:
    QBrush _backgroundBrush;
};

WorkingArea::WorkingArea(MainWindow* window) : QGraphicsView(window)
{
    _item = NULL;
    _window = window;
    setScene(&_scene);
}

QGraphicsItem* WorkingArea::mainItem() const
{
    return _item;
}

void WorkingArea::open(const QImage& image)
{
    selectedTool()->reset();
    resetTransform();
    _scene.clear();
    _scene.setSceneRect(image.rect());
    _item = new WorkingAreaItem(QPixmap::fromImage(image));
    _scene.addItem(_item);
}

Tool* WorkingArea::selectedTool() const 
{ 
    return _window->selectedTool();
}

//////////////////////////////////////////////////////////////////////////

void WorkingArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!selectedTool()->mousePressEvent(event))
        QGraphicsView::mouseDoubleClickEvent(event);
}
void WorkingArea::mousePressEvent(QMouseEvent *event)
{
    if (!selectedTool()->mousePressEvent(event))
        QGraphicsView::mousePressEvent(event);
}
void WorkingArea::mouseMoveEvent(QMouseEvent *event)
{
    if (!selectedTool()->mouseMoveEvent(event))
        QGraphicsView::mouseMoveEvent(event);
}
void WorkingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (!selectedTool()->mouseReleaseEvent(event))
        QGraphicsView::mouseReleaseEvent(event);
}
void WorkingArea::wheelEvent(QWheelEvent *event)
{
    if (!selectedTool()->wheelEvent(event))
        QGraphicsView::wheelEvent(event);
}

//////////////////////////////////////////////////////////////////////////

void WorkingArea::zoom(const QPoint& center, double factor)
{
    if (_item == NULL)
        return;
    setTransform(transform().scale(factor, factor));
    centerOn(mapToScene(center));
}

void WorkingArea::processPolygon(const QPolygonF& polygon)
{
    // TODO
}