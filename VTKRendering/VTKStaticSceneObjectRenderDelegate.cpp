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
#include "Core/StaticSceneObject.h"
#include "VTKRendering/VTKRenderDelegate.h"
#include "Geometry/PlaneModel.h"
#include "Geometry/MeshModel.h"

// VTK includes
#include <vtkActor.h>

class StaticSceneObjectRenderDelegate : public VTKRenderDelegate
{
public:
    vtkActor *getActor() override;
    void draw() const
    { }
};

vtkActor *StaticSceneObjectRenderDelegate::getActor()
{
    StaticSceneObject* geom = this->getSourceGeometryAs<StaticSceneObject>();
    if (!geom)
    {
        return nullptr;
    }

    auto planeModel = std::dynamic_pointer_cast<PlaneModel>(
        geom->getModel());

    if(planeModel)
    {
        auto delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(
            planeModel->getPlaneModel()->getRenderDelegate());
        if(!delegate)
        {
            return nullptr;
        }
        delegate->initDraw();
        return delegate->getActor();
    }

    auto meshModel = std::dynamic_pointer_cast<MeshModel>(
        geom->getModel());

    if(meshModel)
    {
        auto delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(
            meshModel->getMesh()->getRenderDelegate());
        if(!delegate)
        {
            return nullptr;
        }
        delegate->initDraw();
        return delegate->getActor();
    }
    return nullptr;
}

RegisterFactoryClass(RenderDelegate,
                     StaticSceneObjectRenderDelegate,
                     RenderDelegate::RendererType::VTK)
