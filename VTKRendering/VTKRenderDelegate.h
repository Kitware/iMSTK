#ifndef VTKRENDERABLE_H
#define VTKRENDERABLE_H

#include "Core/RenderDelegate.h"

class vtkActor;

class VTKRenderDelegate : public RenderDelegate
{
public:
    virtual vtkActor *getActor() = 0;
    virtual void initDraw() override{}
    virtual void modified() override{}
    virtual void draw() const override
    { }
};

#endif // SMRENDERABLE_H
