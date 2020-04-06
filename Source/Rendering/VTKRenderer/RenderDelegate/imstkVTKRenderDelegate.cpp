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
#include "imstkVolumeRenderMaterial.h"

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
#include "imstkVTKImageDataRenderDelegate.h"

#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLVertexBufferObject.h"
#include "vtkPolyDataNormals.h"
#include "vtkTriangleMeshPointNormals.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkProp3D.h"

namespace imstk
{
std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::makeDelegate(std::shared_ptr<VisualModel> visualModel)
{
    switch (visualModel->getGeometry()->getType())
    {
    case Geometry::Type::Plane:
    {
        return std::make_shared<VTKPlaneRenderDelegate>(visualModel);
    }
    case Geometry::Type::Sphere:
    {
        return std::make_shared<VTKSphereRenderDelegate>(visualModel);
    }
    case Geometry::Type::Capsule:
    {
        return std::make_shared<VTKCapsuleRenderDelegate>(visualModel);
    }
    case Geometry::Type::Cube:
    {
        return std::make_shared<VTKCubeRenderDelegate>(visualModel);
    }
    case Geometry::Type::Cylinder:
    {
        return std::make_shared<VTKCylinderRenderDelegate>(visualModel);
    }
    case Geometry::Type::PointSet:
    {
        return std::make_shared<VTKPointSetRenderDelegate>(visualModel);
    }
    case Geometry::Type::SurfaceMesh:
    {
        return std::make_shared<VTKSurfaceMeshRenderDelegate>(visualModel);
    }
    case Geometry::Type::TetrahedralMesh:
    {
        return std::make_shared<VTKTetrahedralMeshRenderDelegate>(visualModel);
    }
    case Geometry::Type::LineMesh:
    {
        return std::make_shared<VTKLineMeshRenderDelegate>(visualModel);
    }
    case Geometry::Type::HexahedralMesh:
    {
        return std::make_shared<VTKHexahedralMeshRenderDelegate>(visualModel);
    }
    case Geometry::Type::ImageData:
    {
        return std::make_shared<VTKImageDataRenderDelegate>(visualModel);
    }
    default:
    {
        LOG(FATAL) << "RenderDelegate::makeDelegate error: Geometry type incorrect.";
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
VTKRenderDelegate::setUpMapper(vtkAlgorithmOutput*             source,
                               const bool                      notSurfaceMesh,
                               std::shared_ptr<RenderMaterial> renderMat)
{
    if (auto imData = vtkImageData::SafeDownCast(source->GetProducer()->GetOutputDataObject(0)))
    {
        m_volumeMapper->SetInputConnection(source);
        m_modelIsVolume = true;
        return;
    }
    else
    {
        m_modelIsVolume = false;
    }
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
    if (auto mapper = VTKCustomPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
    {
        mapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);

        /*if (!renderMat)
        {
            geometry->setRenderMaterial(std::make_shared<RenderMaterial>());
        }*/

        mapper->setRenderMaterial(renderMat);
    }
}

vtkSmartPointer<vtkProp3D>
VTKRenderDelegate::getVtkActor() const
{
    if (m_modelIsVolume)
    {
        return m_volume;
    }
    else
    {
        return m_actor;
    }
}

void
VTKRenderDelegate::update()
{
    this->updateDataSource();
    this->updateActorTransform();
    this->updateActorProperties();

    vtkSmartPointer<vtkProp3D> actor = nullptr;
    if (m_modelIsVolume)
    {
        actor = m_volume;
    }
    else
    {
        actor = m_actor;
    }
    if (m_visualModel->isVisible())
    {
        actor->VisibilityOn();
        return;
    }
    else
    {
        actor->VisibilityOff();
        return;
    }
}

void
VTKRenderDelegate::updateActorTransform()
{
    auto geom = m_visualModel->getGeometry();
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
    if (!m_visualModel)
    {
        return;
    }
    auto material = m_visualModel->getRenderMaterial();

    if (!material || !material->m_modified)
    {
        return;
    }
    if (VolumeRenderMaterial* volumeMat = dynamic_cast<VolumeRenderMaterial*>(material.get()))
    {
        switch (volumeMat->getBlendMode())
        {
        case RenderMaterial::BlendMode::Alpha:
            m_volumeMapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
            break;
        case RenderMaterial::BlendMode::Additive:
            m_volumeMapper->SetBlendMode(vtkVolumeMapper::ADDITIVE_BLEND);
            break;
        case RenderMaterial::BlendMode::MaximumIntensity:
            m_volumeMapper->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
            break;
        case RenderMaterial::BlendMode::MinimumIntensity:
            m_volumeMapper->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
            break;
        default:
            m_volumeMapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
            break;
        }
        m_volume->SetProperty(volumeMat->getVolumeProperty());
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
    case RenderMaterial::DisplayMode::Wireframe:
        actorProperty->SetRepresentationToWireframe();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::Points:
        actorProperty->SetRepresentationToPoints();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::WireframeSurface:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(true);
        break;
    case RenderMaterial::DisplayMode::Surface:
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
}
} // imstk
