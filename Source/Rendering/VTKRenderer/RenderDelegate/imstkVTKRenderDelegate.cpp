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
#include "imstkCylinder.h"
#include "imstkCube.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkVTKdebugTrianglesRenderDelegate.h"
#include "imstkVTKdebugLinesRenderDelegate.h"
#include "imstkVTKdebugPointsRenderDelegate.h"

// VTK render delegates
#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkVTKSphereRenderDelegate.h"
#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkVTKCubeRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkVTKTetrahedralMeshRenderDelegate.h"
#include "imstkVTKHexahedralMeshRenderDelegate.h"
#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkVTKPointSetRenderDelegate.h"

#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLVertexBufferObject.h"
#include "vtkPolyDataNormals.h"
#include "vtkTriangleMeshPointNormals.h"
#include "vtkTransform.h"

namespace imstk
{
std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::makeDelegate(std::shared_ptr<Geometry> geom)
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
        auto capsule = std::dynamic_pointer_cast<Capsule>(geom);
        return std::make_shared<VTKCapsuleRenderDelegate>(capsule);
    }
    case Geometry::Type::Cube:
    {
        auto cube = std::dynamic_pointer_cast<Cube>(geom);
        return std::make_shared<VTKCubeRenderDelegate>(cube);
    }
    case Geometry::Type::Cylinder:
    {
        auto cylinder = std::dynamic_pointer_cast<Cylinder>(geom);
        return std::make_shared<VTKCylinderRenderDelegate>(cylinder);
    }
    case Geometry::Type::PointSet:
    {
        auto pointSet = std::dynamic_pointer_cast<PointSet>(geom);
        return std::make_shared<VTKPointSetRenderDelegate>(pointSet);
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
        LOG(WARNING) << "RenderDelegate::makeDelegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

std::shared_ptr<imstk::VTKRenderDelegate>
VTKRenderDelegate::makeDebugDelegate(std::shared_ptr<DebugRenderGeometry> geom)
{
    switch (geom->getType())
    {

    case DebugRenderGeometry::Type::Points:
    {
        auto points = std::dynamic_pointer_cast<DebugRenderPoints>(geom);
        return std::make_shared<VTKdbgPointsRenderDelegate>(points);
    }
    case DebugRenderGeometry::Type::Lines:
    {
        auto lines = std::dynamic_pointer_cast<DebugRenderLines>(geom);
        return std::make_shared<VTKdbgLinesRenderDelegate>(lines);
    }
    case DebugRenderGeometry::Type::Triangles:
    {
        auto triangles = std::dynamic_pointer_cast<DebugRenderTriangles>(geom);
        return std::make_shared<VTKdbgTrianglesRenderDelegate>(triangles);
    }
    default:
    {
        LOG(WARNING) << "RenderDelegate::makeDelegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

void
VTKRenderDelegate::setUpMapper(vtkAlgorithmOutput *source,
                               const bool notSurfaceMesh,
                               std::shared_ptr<RenderMaterial> renderMat)
{
    // Add normals
    if (notSurfaceMesh)
    {
        vtkSmartPointer<vtkPolyDataAlgorithm> normalGen;
        normalGen = vtkSmartPointer<vtkPolyDataNormals>::New();
        vtkPolyDataNormals::SafeDownCast(normalGen)->SplittingOff();
        normalGen->SetInputConnection(source);
        m_mapper->SetInputConnection(normalGen->GetOutputPort());
    }
    else
    {
        m_mapper->SetInputConnection(source);
    }

    // Disable auto Shift & Scale which is slow for deformable objects
    // as it needs to compute a bounding box at every frame
    if(auto mapper = VTKCustomPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
    {
        mapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);

        /*if (!renderMat)
        {
            geometry->setRenderMaterial(std::make_shared<RenderMaterial>());
        }*/

        mapper->setRenderMaterial(renderMat);
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
    if (!geom || !geom->m_transformModified)
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
    auto material = this->getRenderMaterial();

    if (!material || !material->m_modified)
    {
        return;
    }

    auto actorProperty = m_actor->GetProperty();

    // Colors & Light
    auto diffuseColor = material->m_color;
    actorProperty->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b);

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
        if (material->m_flatShading)
        {
            actorProperty->SetInterpolationToFlat();
        }
        else
        {
            actorProperty->SetInterpolationToGouraud();
        }
        break;
    }

    // Display properties
    actorProperty->SetLineWidth(material->m_lineWidth);
    actorProperty->SetPointSize(material->m_pointSize);
    actorProperty->SetBackfaceCulling(material->m_backfaceCulling);

    // Material state is now up to date
    material->m_stateModified = false;

    if (material->getVisibility())
    {
        m_actor->VisibilityOn();
        return;
    }
    else
    {
        m_actor->VisibilityOff();
        return;
    }
}
} // imstk
