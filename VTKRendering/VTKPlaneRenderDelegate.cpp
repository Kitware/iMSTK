// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/Model.h"
#include "Core/Geometry.h"

// VTK incudes
#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlaneSource.h>
#include "VTKRendering/VTKRenderDelegate.h"

class PlaneRenderDelegate : public VTKRenderDelegate
{
public:
    vtkActor *getActor() const override;
    virtual void initDraw() override;

private:
    vtkNew<vtkActor> actor;
};

void PlaneRenderDelegate::initDraw()
{
    auto plane = this->getSourceGeometryAs<Plane>();
    if(!plane)
    {
        return;
    }

    vtkNew<vtkPlaneSource> planeSource;
    planeSource->SetCenter(-100,-100,0);
    planeSource->SetPoint1(100,-100,0);
    planeSource->SetPoint2(-100,100,0);

    auto center = plane->getPoint();
    auto normal = plane->getUnitNormal();

    planeSource->SetCenter(center(0),center(1),center(2));
    planeSource->SetNormal(normal(0),normal(1),normal(2));

    auto mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(planeSource->GetOutputPort());

    this->actor->SetMapper(mapper);
}

vtkActor *PlaneRenderDelegate::getActor() const
{
    return actor.GetPointer();
}

RegisterFactoryClass(RenderDelegate,
                     PlaneRenderDelegate,
                     RenderDelegate::RendererType::VTK)
