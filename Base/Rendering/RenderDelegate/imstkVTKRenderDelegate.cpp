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
    this->updateDataSource();
    this->updateActorTransform();
    this->updateActorProperties();
}

void
VTKRenderDelegate::updateActorTransform()
{
    auto geom = this->getGeometry();
    if (!geom->m_transformModified)
    {
        return;
    }
    AffineTransform3d T(geom->m_transform.matrix());
    T.scale(geom->getScaling());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
    geom->m_transformModified = false;
}

void
VTKRenderDelegate::updateActorProperties()
{
    auto material = this->getGeometry()->getRenderMaterial();

    if (!material || !material->m_modified)
    {
        return;
    }

    auto actorProperty = m_actor->GetProperty();

    // Colors & Light
    auto diffuseColor = material->m_diffuseColor;
    auto specularColor = material->m_specularColor;
    auto specularity = material->m_specularity;
    actorProperty->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    actorProperty->SetSpecularColor(specularColor.r, specularColor.g, specularColor.b);
    actorProperty->SetSpecularPower(specularity);
    actorProperty->SetSpecular(1.0);

    // Material state is now up to date
    material->m_modified = false;

    if (!material->m_stateModified)
    {
        return;
    }

    // Display mode
    switch (material->m_displayMode)
    {
    case RenderMaterial::DisplayMode::WIREFRAME:
        actorProperty->SetRepresentationToWireframe();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::POINTS:
        actorProperty->SetRepresentationToPoints();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::WIREFRAME_SURFACE:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(true);
        break;
    case RenderMaterial::DisplayMode::SURFACE:
    default:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(false);
        break;
    }

    // Display properties
    actorProperty->SetLineWidth(material->m_lineWidth);
    actorProperty->SetPointSize(material->m_pointSize);
    actorProperty->SetBackfaceCulling(material->m_backfaceCulling);

    // Material state is now up to date
    material->m_stateModified = false;
}

} // imstk
