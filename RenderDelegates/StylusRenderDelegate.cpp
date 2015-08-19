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
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Rendering/GLUtils.h"
#include "Simulators/StylusObject.h"

class StylusRenderDelegate : public RenderDelegate
{
public:
  virtual void initDraw() override;
  virtual void draw() const override;
};

void StylusRenderDelegate::initDraw()
{
  StylusRigidSceneObject* geom = this->getSourceGeometryAs<StylusRigidSceneObject>();
  std::string errorText;
  tree<MeshContainer*>::pre_order_iterator iter = geom->meshes.begin();
  GLint newList = glGenLists(geom->meshes.size());
  GLUtils::queryGLError(errorText);

  int listCounter = 0;

  while (iter != geom->meshes.end())
    {
    glNewList(newList + listCounter, GL_COMPILE);
    iter.node->data->mesh->draw();
    glEndList();
    iter.node->data->mesh->setRenderingId(newList + listCounter);
    listCounter++;
    iter++;
    }
}

void StylusRenderDelegate::draw() const
{
  StylusRigidSceneObject* geom = this->getSourceGeometryAs<StylusRigidSceneObject>();
  Matrix44d viewMatrix;

  for (int i = 0; i < 2; i++)
    {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tree<MeshContainer*>::pre_order_iterator iter = geom->meshes.begin();
    glPushMatrix();

    if (i == 2 && geom->enableDeviceManipulatedTool)
      {
      viewMatrix = iter.node->data->currentDeviceMatrix;
      }
    else
      {
      viewMatrix = iter.node->data->currentViewerMatrix;
      }

    glMultMatrixd(viewMatrix.data());
    glCallList(iter.node->data->mesh->getRenderingId());
    glPopMatrix();
    iter++;

    while (iter != geom->meshes.end())
      {
      glPushMatrix();

      if (i == 2 && geom->enableDeviceManipulatedTool)
        {
        viewMatrix = iter.node->data->currentDeviceMatrix;
        }
      else
        {
        viewMatrix = iter.node->data->currentViewerMatrix;
        }

      glMultMatrixd(viewMatrix.data());
      glCallList(iter.node->data->mesh->getRenderingId());
      glPopMatrix();
      iter++;
      }

    glPopMatrix();
    }
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_stylus_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,StylusRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
