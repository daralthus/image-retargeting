#pragma once

class WorkItem
{
public:
    virtual ~WorkItem() {}

    virtual void execute() = 0;
};