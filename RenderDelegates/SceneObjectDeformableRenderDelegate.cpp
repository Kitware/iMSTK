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

#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Simulators/SceneObjectDeformable.h"

/// \brief  Displays the fem object with primary or secondary mesh, fixed vertices,
///  vertices interacted with, ground plane etc.
class SceneObjectDeformableRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void SceneObjectDeformableRenderDelegate::draw() const
{
  auto geom = this->getSourceGeometryAs<SceneObjectDeformable>();
  if (!geom)
    return;

  if (geom->renderSecondaryMesh && !!geom->getSecondarySurfaceMesh())
    {
    geom->getSecondarySurfaceMesh()->draw();
    }
  else
    {
    geom->getPrimarySurfaceMesh()->draw();
    }
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_scene_object_deformable_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,SceneObjectDeformableRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
