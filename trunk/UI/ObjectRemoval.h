#pragma once

#include "WorkItem.h"
#include "WorkingArea.h"

#include "../IRL/Image.h"
#include "../IRL/ImageWithMask.h"
#include "../IRL/IO.h"
#include "../IRL/RGB.h"
#include "../IRL/Lab.h"
#include "../IRL/ObjectRemoval.h"

typedef IRL::LabDouble Color;

class ObjectRemovalWorkItem :
    public WorkItem,
    public IRL::OperationCallback<Color>
{
public:
    ObjectRemovalWorkItem(WorkingArea* workingArea, const QImage& image, 
        const QPolygonF& mask, qreal scaleX, qreal scaleY);

    virtual void execute();

    virtual void IntermediateResult(const IRL::Image<Color>& result, int progress, int total);
    virtual void OperationEnded(const IRL::Image<Color>& result);

private:
    void prepareMask(QImage& surface);
    void pushUpdate(const QImage& image, int progress, bool final);

private:
    QImage _image;
    QPolygonF _poly;
    qreal _scaleX, _scaleY;
    clock_t _lastResultTime;
    WorkingArea* _workingArea;
};