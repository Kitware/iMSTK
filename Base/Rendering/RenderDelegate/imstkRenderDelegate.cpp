/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkRenderDelegate.h"

#include "g3log/g3log.hpp"

#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkPlaneRenderDelegate.h"
#include "imstkSphereRenderDelegate.h"
#include "imstkCubeRenderDelegate.h"
#include "imstkSurfaceMeshRenderDelegate.h"
#include "imstkTetrahedralMeshRenderDelegate.h"

#include "vtkPolyDataMapper.h"
#include "vtkTransform.h"

namespace imstk {
std::shared_ptr<RenderDelegate>
RenderDelegate::make_delegate(std::shared_ptr<Geometry>geom)
{
    switch (geom->getType())
    {
    case GeometryType::Plane:
    {
        auto plane = std::dynamic_pointer_cast<Plane>(geom);
        return std::make_shared<PlaneRenderDelegate>(plane);
    }
    case GeometryType::Sphere:
    {
        auto sphere = std::dynamic_pointer_cast<Sphere>(geom);
        return std::make_shared<SphereRenderDelegate>(sphere);
    }
    case GeometryType::Cube:
    {
        auto cube = std::dynamic_pointer_cast<Cube>(geom);
        return std::make_shared<CubeRenderDelegate>(cube);
    }
    case GeometryType::SurfaceMesh:
    {
        auto surface = std::dynamic_pointer_cast<SurfaceMesh>(geom);
        return std::make_shared<SurfaceMeshRenderDelegate>(surface);
    }
    case GeometryType::TetrahedralMesh:
    {
        auto mesh = std::dynamic_pointer_cast<TetrahedralMesh>(geom);
        return std::make_shared<TetrahedralMeshRenderDelegate>(mesh);
    }
    case GeometryType::HexahedralMesh:
    {
        auto mesh = std::dynamic_pointer_cast<TetrahedralMesh>(geom);
        LOG(WARNING) << "RenderDelegate::make_delegate error: HexahedralMeshRenderDelegate not yet implemented";
        return nullptr;
    }
    default:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

void
RenderDelegate::setActorMapper(vtkAlgorithmOutput *source)
{
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    mapper->SetInputConnection(source);

    m_actor->SetMapper(mapper);
}

void
RenderDelegate::setActorTransform(std::shared_ptr<Geometry>geom)
{
    auto scaling   = geom->getScaling();
    auto quat      = geom->getOrientation();
    auto angleAxis = Rotd(quat);
    auto pos       = geom->getPosition();

    auto transform = vtkSmartPointer<vtkTransform>::New();

    transform->PostMultiply();
    transform->Scale(scaling, scaling, scaling);
    transform->RotateWXYZ(angleAxis.angle() * 180 / M_PI,
                          angleAxis.axis()[0],
                          angleAxis.axis()[1],
                          angleAxis.axis()[2]);
    transform->Translate(pos[0], pos[1], pos[2]);

    m_actor->SetUserTransform(transform);
}

vtkSmartPointer<vtkActor>
RenderDelegate::getVtkActor() const
{
    return m_actor;
}
}
