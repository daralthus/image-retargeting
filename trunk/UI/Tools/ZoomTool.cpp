#include "../Includes.h"
#include "../MainWindow.h"
#include "../WorkingArea.h"
#include "ZoomTool.h"

ZoomTool::ZoomTool(MainWindow* window, ZoomType type) : Tool(window)
{
    _type = type;
    if (type == ZoomIn)
    {
        initialize("Zoom In", QIcon(":/images/zoom-in.png"), QCursor(QPixmap(":/images/zoom-in-cursor.png")));
        action()->setShortcut(QKeySequence(Qt::Key_Z));
    } else
    {
        initialize("Zoom Out", QIcon(":/images/zoom-out.png"), QCursor(QPixmap(":/images/zoom-out-cursor.png")));
        action()->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Z));
    }
}

bool ZoomTool::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return false;
    bool zoomIn = (_type == ZoomIn);
    if (e->button() == Qt::RightButton)
        zoomIn = !zoomIn;
    mainWindow()->workingArea()->zoom(e->pos(), zoomIn ? 1.5 : 1 / 1.5);
    return true;
}

bool ZoomTool::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return false;
    return true;
}