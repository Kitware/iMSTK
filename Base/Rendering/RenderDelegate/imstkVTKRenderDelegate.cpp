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

#include "imstkVTKRenderDelegate.h"

#include "g3log/g3log.hpp"

#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCapsule.h"
#include "imstkCube.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkVTKSphereRenderDelegate.h"
#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkVTKCubeRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkVTKTetrahedralMeshRenderDelegate.h"
#include "imstkVTKHexahedralMeshRenderDelegate.h"

#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLVertexBufferObject.h"
#include "vtkPolyDataNormals.h"
#include "vtkTriangleMeshPointNormals.h"
#include "vtkTransform.h"

namespace imstk
{

std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::make_delegate(std::shared_ptr<Geometry>geom)
{
    switch (geom->getType())
    {
    case Geometry::Type::Plane:
    {
        auto plane = std::dynamic_pointer_cast<Plane>(geom);
        return std::make_shared<VTKPlaneRenderDelegate>(plane);
    }
    case Geometry::Type::Sphere:
    {
        auto sphere = std::dynamic_pointer_cast<Sphere>(geom);
        return std::make_shared<VTKSphereRenderDelegate>(sphere);
    }
    case Geometry::Type::Capsule:
    {
        auto sphere = std::dynamic_pointer_cast<Capsule>(geom);
        return std::make_shared<VTKCapsuleRenderDelegate>(sphere);
    }
    case Geometry::Type::Cube:
    {
        auto cube = std::dynamic_pointer_cast<Cube>(geom);
        return std::make_shared<VTKCubeRenderDelegate>(cube);
    }
    case Geometry::Type::SurfaceMesh:
    {
        auto surface = std::dynamic_pointer_cast<SurfaceMesh>(geom);
        return std::make_shared<VTKSurfaceMeshRenderDelegate>(surface);
    }
    case Geometry::Type::TetrahedralMesh:
    {
        auto mesh = std::dynamic_pointer_cast<TetrahedralMesh>(geom);
        return std::make_shared<VTKTetrahedralMeshRenderDelegate>(mesh);
    }
    case Geometry::Type::LineMesh:
    {
        auto mesh = std::dynamic_pointer_cast<LineMesh>(geom);
        return std::make_shared<VTKLineMeshRenderDelegate>(mesh);
    }
    case Geometry::Type::HexahedralMesh:
    {
        auto mesh = std::dynamic_pointer_cast<HexahedralMesh>(geom);
        return std::make_shared<VTKHexahedralMeshRenderDelegate>(mesh);
    }
    default:
    {
        LOG(WARNING) << "RenderDelegate::make_delegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

void
VTKRenderDelegate::setUpMapper(vtkAlgorithmOutput *source, const bool rigid)
{
    // Add normals
    vtkSmartPointer<vtkPolyDataAlgorithm> normalGen;
    if (rigid)
    {
        normalGen = vtkSmartPointer<vtkPolyDataNormals>::New();
        vtkPolyDataNormals::SafeDownCast(normalGen)->SplittingOff();
    }
    else
    {
        normalGen = vtkSmartPointer<vtkTriangleMeshPointNormals>::New();
    }
    normalGen->SetInputConnection(source);

    m_mapper->SetInputConnection(normalGen->GetOutputPort());

    // Disable auto Shift & Scale which is slow for deformable objects
    // as it needs to compute a bounding box at every frame
    if(auto mapper = vtkOpenGLPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
    {
        mapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
    }
}

vtkSmartPointer<vtkActor>
VTKRenderDelegate::getVtkActor() const
{
    return m_actor;
}

void
VTKRenderDelegate::update()
{
    // TODO : only when rigid transform applied
    this->updateActorTransform();
}

void
VTKRenderDelegate::updateActorTransform()
{
    auto scaling   = this->getGeometry()->getScaling();
    auto pos       = this->getGeometry()->getPosition();
    auto quat      = this->getGeometry()->getOrientation();
    auto angleAxis = Rotd(quat);

    m_transform->Identity();
    m_transform->PostMultiply();
    m_transform->Scale(scaling, scaling, scaling);
    m_transform->RotateWXYZ(angleAxis.angle() * 180 / PI,
                          angleAxis.axis()[0],
                          angleAxis.axis()[1],
                          angleAxis.axis()[2]);
    m_transform->Translate(pos[0], pos[1], pos[2]);
}

} // imstk
