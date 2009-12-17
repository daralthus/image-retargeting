#pragma once

#include "../Tool.h"

class ZoomTool : public Tool
{
public:
    enum ZoomType
    {
        ZoomIn,
        ZoomOut
    };

    ZoomTool(MainWindow* window, ZoomType type);

    virtual bool mousePressEvent(QMouseEvent*);
    virtual bool mouseReleaseEvent(QMouseEvent*);

private:
    ZoomType _type;
};