#pragma once

class MainWindow;
class Tool;
class WorkingAreaItem;

class WorkingArea :
    public QGraphicsView
{
    Q_OBJECT

    friend class PixmapItem;
public:
    WorkingArea(MainWindow* window);

public slots:
    void open(const QImage& image);
    void zoom(const QPoint& center, double factor);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    Tool* selectedTool() const;

private:
    MainWindow* _window;
    QGraphicsScene _scene;
    WorkingAreaItem* _item;
};