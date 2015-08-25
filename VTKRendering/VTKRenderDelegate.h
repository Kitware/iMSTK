#ifndef VTKRENDERABLE_H
#define VTKRENDERABLE_H

#include "Core/RenderDelegate.h"

class vtkActor;

class VTKRenderDelegate : public RenderDelegate
{
public:
    virtual vtkActor *getActor() const = 0;
    virtual void initDraw(){}
    virtual void modified(){}
};

#endif // SMRENDERABLE_H
