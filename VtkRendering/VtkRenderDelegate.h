#ifndef VTKRENDERABLE_H
#define VTKRENDERABLE_H

#include "Core/RenderDelegate.h"

class vtkActor;

class VtkRenderDelegate : public RenderDelegate
{
public:
    virtual vtkActor *getActor() const = 0;
    virtual void initDraw(){};
};

#endif // SMRENDERABLE_H
