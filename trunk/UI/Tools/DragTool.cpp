#include "../Includes.h"
#include "../MainWindow.h"
#include "../WorkingArea.h"

#include "DragTool.h"

DragTool::DragTool(MainWindow* window) : Tool(window)
{
    initialize("Drag", QIcon(":/images/drag.png"), QCursor(Qt::ArrowCursor));
    action()->setShortcut(QKeySequence(Qt::Key_Space));
}

void DragTool::selected()
{
    mainWindow()->workingArea()->setDragMode(QGraphicsView::ScrollHandDrag);
}

void DragTool::unselected()
{
    mainWindow()->workingArea()->setDragMode(QGraphicsView::NoDrag);
}