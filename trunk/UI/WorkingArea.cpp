#include "Includes.h"
#include "WorkingArea.h"
#include "MainWindow.h"
#include "ObjectRemoval.h"

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

    connect(&_checker, SIGNAL(timeout()), this, SLOT(checkUpdateQueue()));
}

QGraphicsItem* WorkingArea::mainItem() const
{
    return _item;
}

void WorkingArea::open(const QImage& image)
{
    _originalSize = image.size();
    double lnw = ceil(log((double)image.width()) / log(2.0));
    _powerOfTwoSize.setWidth(pow(2, lnw));
    double lnh = ceil(log((double)image.height()) / log(2.0));
    _powerOfTwoSize.setHeight(pow(2, lnh));

    _scaleX = qreal(_originalSize.width()) / _powerOfTwoSize.width();
    _scaleY = qreal(_originalSize.height()) / _powerOfTwoSize.height();

    _workingCopy = image.scaled(_powerOfTwoSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    selectedTool()->reset();
    resetTransform();
    _scene.clear();
    _item = new WorkingAreaItem(QPixmap::fromImage(_workingCopy));
    _item->setTransform(QTransform::fromScale(_scaleX, _scaleY));
    _scene.setSceneRect(0, 0, _originalSize.width(), _originalSize.height());
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
    qreal scaleX = qreal(_powerOfTwoSize.width()) / _originalSize.width();
    qreal scaleY = qreal(_powerOfTwoSize.height()) / _originalSize.height();

    _window->setBusy(true);
    _window->setProgress(true, 0, 100);
    _checker.start(50);
    _window->enqueueWorkItem(new ObjectRemovalWorkItem(this, _workingCopy, polygon, scaleX, scaleY));
}

void WorkingArea::pushUpdate(const QImage& img, const QPolygonF& mask, int progress, bool final)
{
    QMutexLocker locker(&_updatesLock);
    _updates.push_back(Update(img, mask, progress, final));
}

void WorkingArea::checkUpdateQueue()
{
    _updatesLock.lock();
    if (_updates.empty())
    {
        _updatesLock.unlock();
        return;
    }
    Update update = _updates.front();
    _updates.pop_front();
    _updatesLock.unlock();

    displayUpdate(update);
    if (update.Final)
        _checker.stop();
}

void WorkingArea::displayUpdate(const Update& update)
{
    if (update.Final)
    {
        _workingCopy = update.Image;
        _item->setPixmap(QPixmap::fromImage(_workingCopy));
        _item->setTransform(QTransform::fromScale(_scaleX, _scaleY));
        _window->setBusy(false);
        _window->setProgress(false, 100, 100);
    } else
    {
        _item->setPixmap(QPixmap::fromImage(update.Image));
        _item->setTransform(QTransform::fromScale(_powerOfTwoSize.width() / update.Image.width(), 
            _powerOfTwoSize.height() / update.Image.height()));
        _item->setTransform(QTransform::fromScale(_scaleX, _scaleY), true);
        _window->setProgress(true, update.Progress, 100);
    }
}