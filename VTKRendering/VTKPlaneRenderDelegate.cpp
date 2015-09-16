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
#include <vtkProperty.h>
#include <vtkJPEGReader.h>
#include <vtkFloatArray.h>
#include "VTKRendering/VTKRenderDelegate.h"

class PlaneRenderDelegate : public VTKRenderDelegate
{
public:
    vtkActor *getActor() override;
    virtual void initDraw() override;
    void draw() const
    { }

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

    auto renderDetail = plane->getRenderDetail();
    if(renderDetail)
    {
        auto ambientColor = renderDetail->getColorAmbient().getValue();
        auto diffuseColor = renderDetail->getColorDiffuse().getValue();
        auto specularColor = renderDetail->getColorSpecular().getValue();
        auto specularPower = renderDetail->getShininess();
        auto opacity = renderDetail->getOpacity();
        actor->GetProperty()->SetAmbient(ambientColor[3]);
        actor->GetProperty()->SetAmbientColor(ambientColor[0],ambientColor[1],ambientColor[2]);
        actor->GetProperty()->SetDiffuse(diffuseColor[3]);
        actor->GetProperty()->SetDiffuseColor(diffuseColor[0],diffuseColor[1],diffuseColor[2]);
        actor->GetProperty()->SetSpecular(specularColor[3]);
        actor->GetProperty()->SetSpecularColor(specularColor[0],specularColor[1],specularColor[2]);
        actor->GetProperty()->SetSpecularPower(specularPower);
        actor->GetProperty()->SetOpacity(opacity);
    }
    if(renderDetail && renderDetail->renderWireframe())
    {
        actor->GetProperty()->SetRepresentationToWireframe();
    }

    vtkNew<vtkPlaneSource> planeSource;
    auto center = plane->getPoint();
    auto normal = plane->getUnitNormal();
    core::Vec3d xp = normal.cross(core::Vec3d(0., 1., 0.));
    if (xp.squaredNorm() < 1e-8)
      xp = normal.cross(core::Vec3d(0., 0., 1.)).normalized();
    else
      xp = xp.normalized();
    core::Vec3d yp = normal.cross(xp).normalized();

    double planeSize = plane->getWidth();
    xp *= planeSize;
    yp *= planeSize;

    planeSource->SetCenter((center-xp-yp)(0),(center-xp-yp)(1),(center-xp-yp)(2));
    planeSource->SetPoint1((center+xp-yp)(0),(center+xp-yp)(1),(center+xp-yp)(2));
    planeSource->SetPoint2((center-xp+yp)(0),(center-xp+yp)(1),(center-xp+yp)(2));
    planeSource->SetNormal(normal(0),normal(1),normal(2));

    auto mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(planeSource->GetOutputPort());

    this->actor->SetMapper(mapper);
}

vtkActor *PlaneRenderDelegate::getActor()
{
    return actor.GetPointer();
}

RegisterFactoryClass(RenderDelegate,
                     PlaneRenderDelegate,
                     RenderDelegate::RendererType::VTK)
