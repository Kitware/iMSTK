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
#include "Core/Geometry.h" // for VisualArtifact used by geometry sources in RenderDelegate
#include "Simulators/VegaFemSceneObject.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "VTKRendering/VTKRenderDelegate.h"

// VTK includes
#include <vtkActor.h>

/// \brief  Displays the fem object with primary or secondary mesh, fixed vertices,
///  vertices interacted with, ground plane etc.
class SceneObjectDeformableRenderDelegate : public VTKRenderDelegate
{
public:
    vtkActor *getActor() override;
    void draw() const
    { }
};

vtkActor *SceneObjectDeformableRenderDelegate::getActor()
{
    auto geom = this->getSourceGeometryAs<VegaFemSceneObject>();

    if(!geom)
    {
        return nullptr;
    }
    auto mesh = geom->getVolumetricMesh();
    if(!mesh)
    {
        return nullptr;
    }
    auto surfaceMesh = mesh->getRenderingMesh();
    if(!surfaceMesh)
    {
        return nullptr;
    }
    auto delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(surfaceMesh->getRenderDelegate());
    if(!delegate)
    {
        return nullptr;
    }
    delegate->initDraw();
    return delegate->getActor();
}

RegisterFactoryClass(RenderDelegate,
                     SceneObjectDeformableRenderDelegate,
                     RenderDelegate::RendererType::VTK)