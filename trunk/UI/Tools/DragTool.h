#pragma once

#include "../Tool.h"

class DragTool : public Tool
{
public:
    DragTool(MainWindow* window);

protected:
    virtual void selected();
    virtual void unselected();
};