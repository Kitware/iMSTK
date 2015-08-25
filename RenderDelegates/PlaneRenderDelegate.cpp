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

class PlaneRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void PlaneRenderDelegate::draw() const
{
  OpenGLRenderer::draw(*this->getSourceGeometryAs<Plane>());
//         if (this->movedOrRotated)
//         {
//             updateDrawPoints();
//         }
//         glEnable(GL_LIGHTING);
//
//         glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color::colorGray.toGLColor());
//         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Color::colorWhite.toGLColor());
//         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Color::colorGray.toGLColor());
//
//         glEnable(GL_BLEND);
//         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//         glPushMatrix();
//             glBegin(GL_QUADS);
//                 glVertex3f(this->drawPoints[0][0], this->drawPoints[0][1], this->drawPoints[0][2]);
//                 glVertex3f(this->drawPoints[1][0], this->drawPoints[1][1], this->drawPoints[1][2]);
//                 glVertex3f(this->drawPoints[2][0], this->drawPoints[2][1], this->drawPoints[2][2]);
//                 glVertex3f(this->drawPoints[3][0], this->drawPoints[3][1], this->drawPoints[3][2]);
//             glEnd();
//         glPopMatrix();
//
//         glDisable(GL_BLEND);
//
//         glEnable(GL_LIGHTING);

}

RegisterFactoryClass(RenderDelegate,
                     PlaneRenderDelegate,
                     RenderDelegate::RendererType::Other)
