#pragma once

#include "../Tool.h"

class PolygonItem;
class PolygonTool : public Tool
{
public:
    PolygonTool(MainWindow* window);

    virtual void selected();
    virtual void unselected();
    virtual void reset();

    virtual bool mousePressEvent(QMouseEvent*);
    virtual bool mouseReleaseEvent(QMouseEvent*);

private:
    void startNewPolygon();
    void abandonPolygon();
    void polygonClosed();

private:
    PolygonItem* _item;
    QPolygonF _poly;
    QPainterPath _path;
};