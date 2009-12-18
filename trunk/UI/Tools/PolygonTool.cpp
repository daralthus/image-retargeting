#include "../Includes.h"
#include "../MainWindow.h"
#include "../WorkingArea.h"
#include "PolygonTool.h"

class PolygonItem :
    public QGraphicsPathItem
{
public:
    PolygonItem()
    { }

    void setPoints(const QPolygonF& points) { _points = points; }
    const QPolygonF& points() const { return _points; }

protected:
    virtual QRectF boundingRect() const
    {
        QRectF res = QGraphicsPathItem::boundingRect();
        return res.adjusted(-2, -2, 2, 2);
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QGraphicsPathItem::paint(painter, option, widget);

        QPen oldPen = painter->pen();
        QBrush oldBrush = painter->brush();
        painter->setPen(QPen(qRgb(255, 0, 0)));
        painter->setBrush(QBrush(qRgb(255, 0, 0)));
        QTransform t = painter->transform();
        qreal scaleX = sqrt(t.m11()*t.m11() + t.m12()*t.m12());
        qreal scaleY = sqrt(t.m21()*t.m21() + t.m22()*t.m22());
        QRectF rect = option->exposedRect.adjusted(- 2 / scaleX, - 2 / scaleY, 2 / scaleX, 2 / scaleY);
        for (int i = 0; i < _points.size(); i++)
        {
            if (rect.contains(_points[i]))
                painter->drawEllipse(_points[i], 2 / scaleX, 2 / scaleY);
        }
        painter->setPen(oldPen);
        painter->setBrush(oldBrush);
    }

private:
    QPolygonF _points;
};

PolygonTool::PolygonTool(MainWindow* window) : Tool(window)
{
    _item = NULL;
    initialize("Polygon", QIcon(":/images/polygon.png"), QCursor(Qt::CrossCursor));
    action()->setShortcut(QKeySequence(Qt::Key_P));
    setCategory(1);
}

void PolygonTool::startNewPolygon()
{
    abandonPolygon();
    
    WorkingArea* view = mainWindow()->workingArea();
    QPointF d = view->mapToScene(0, 0) - view->mapToScene(2, 0);
    float width = sqrt(d.x() * d.x() + d.y() * d.y());
    QPen pen;
    pen.setWidthF(width);
    pen.setColor(qRgb(0, 255, 0));

    _item = new PolygonItem();
    _item->setPen(pen);
    mainWindow()->workingArea()->scene().addItem(_item);
    _item->setPos(0, 0);
    _poly.clear();
}

void PolygonTool::abandonPolygon()
{
    if (_item)
        delete _item;
    _item = NULL;
    _poly.clear();
    _path = QPainterPath();
}

void PolygonTool::polygonClosed()
{
    mainWindow()->workingArea()->processPolygon(_poly);
    abandonPolygon();
}

void PolygonTool::selected()
{
    startNewPolygon();
}

void PolygonTool::unselected()
{
    abandonPolygon();
}

void PolygonTool::reset()
{
    abandonPolygon();
}

bool PolygonTool::mousePressEvent(QMouseEvent* e)
{
    WorkingArea* view = mainWindow()->workingArea();
    if (e->button() == Qt::RightButton)
    {
        abandonPolygon();
        return true;
    }
    if (e->button() != Qt::LeftButton || view->mainItem() == NULL)
        return false;
    if (_item == NULL)
        startNewPolygon();
    
    // find point in scene coords
    QPointF pos = view->mapToScene(e->pos());

    // check boundaries
    if (!view->mainItem()->boundingRect().contains(pos))
        return false;

    // try to close the shape
    if (_poly.size() > 0)
    {
        QPoint firstPoint = view->mapFromScene(_poly[0]);
        if ((firstPoint - e->pos()).manhattanLength() < 15)
        {
            // close it!
            _poly.append(_poly[0]);
            polygonClosed();
            return true;
        }
    }

    // append to polygon and path
    _poly.append(pos);
    if (_poly.size() == 1)
        _path.moveTo(pos);
    else
        _path.lineTo(pos);

    _item->setPoints(_poly);
    _item->setPath(_path);
    return true;
}

bool PolygonTool::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
        return false;
    return true;
}