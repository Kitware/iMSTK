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

#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Mesh/Mesh.h"

class MeshRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
  virtual bool isTargetTextured() const override;
};

void MeshRenderDelegate::draw() const
{
  auto geom = this->getSourceGeometryAs<Mesh>();
  if (!geom)
    return;

  auto mesh = std::dynamic_pointer_cast<Mesh>(geom->shared_from_this());
  GLRenderer::drawSurfaceMeshTriangles(mesh, geom->getRenderDetail());

  if (geom->getRenderDetail()->renderType & SIMMEDTK_RENDER_NORMALS)
    {
    GLRenderer::drawNormals(mesh,
      geom->getRenderDetail()->normalColor,
      geom->getRenderDetail()->normalLength);
    }
}

bool MeshRenderDelegate::isTargetTextured() const
{
  auto geom = this->getSourceGeometryAs<Mesh>();
  if (!geom)
    return false;

  return geom->isMeshTextured();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_mesh_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,MeshRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
