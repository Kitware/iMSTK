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

class AABBRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void AABBRenderDelegate::draw() const
{
  AABB* geom = this->getSourceGeometryAs<AABB>();
  if (!geom)
    return;

  glBegin(GL_LINES);
    {
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);

    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);

    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);
    }
  glEnd();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_aabb_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,AABBRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()