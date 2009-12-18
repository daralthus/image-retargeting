#pragma once

class MainWindow;
class Tool;
class WorkingAreaItem;

class WorkingArea :
    public QGraphicsView
{
    Q_OBJECT

    friend class PixmapItem;

    class Update
    {
    public:
        Update(const QImage& img, const QPolygonF& mask, int progress, bool final)
            : Image(img), Mask(mask), Final(final), Progress(progress)
        { }

        QImage Image;
        QPolygonF Mask;
        bool Final;
        int Progress;
    };
public:
    WorkingArea(MainWindow* window);
    QGraphicsScene& scene() { return _scene; }
    const QGraphicsScene& scene() const { return _scene; }
    QGraphicsItem* mainItem() const;

    void pushUpdate(const QImage& img, const QPolygonF& mask, int progress, bool final);

public slots:
    void open(const QImage& image);
    void zoom(const QPoint& center, double factor);
    void processPolygon(const QPolygonF& polygon);

private slots:
    void checkUpdateQueue();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    Tool* selectedTool() const;
    void displayUpdate(const Update& update);

private:
    MainWindow* _window;
    QGraphicsScene _scene;
    WorkingAreaItem* _item;

    QSize _originalSize;
    QSize _powerOfTwoSize;
    qreal _scaleX, _scaleY;
    QImage _workingCopy;
    QTimer _checker;

    QList<Update> _updates;
    QMutex _updatesLock;
};