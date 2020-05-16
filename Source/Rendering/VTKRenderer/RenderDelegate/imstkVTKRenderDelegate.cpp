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

#include "imstkLogger.h"

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

#include <vtkOpenGLVertexBufferObject.h>
#include <vtkTriangleMeshPointNormals.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkImageReader2Factory.h>
#include <vtkPolyDataNormals.h>
#include <vtkImageReader2.h>
#include <vtkTransform.h>
#include <vtkImageData.h>
#include <vtkTexture.h>
#include <vtkVolume.h>
#include <vtkVector.h>
#include <vtkProp3D.h>
#include <vtkActor.h>
#include <vtkNew.h>

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
VTKRenderDelegate::makeDebugDelegate(std::shared_ptr<VisualModel> dbgVizModel)
{
    switch (dbgVizModel->getDebugGeometry()->getType())
    {
    case DebugRenderGeometry::Type::Points:
    {
        return std::make_shared<VTKdbgPointsRenderDelegate>(dbgVizModel);
    }
    case DebugRenderGeometry::Type::Lines:
    {
        return std::make_shared<VTKdbgLinesRenderDelegate>(dbgVizModel);
    }
    case DebugRenderGeometry::Type::Triangles:
    {
        return std::make_shared<VTKdbgTrianglesRenderDelegate>(dbgVizModel);
    }
    default:
    {
        LOG(WARNING) << "RenderDelegate::makeDebugDelegate error: Geometry type incorrect.";
        return nullptr;
    }
    }
}

void
VTKRenderDelegate::setUpMapper(vtkAlgorithmOutput* source,
                               const std::shared_ptr<VisualModel> vizModel)
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
    if (!vizModel->getGeometry()->isMesh())
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
    if (auto mapper = vtkOpenGLPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
    {
        mapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
    }
}

vtkProp3D*
VTKRenderDelegate::getVtkActor() const
{       
    if (m_modelIsVolume)
    {
        return m_volume.GetPointer();
    }
    else
    {
        return m_actor.GetPointer();
    }
}

void
VTKRenderDelegate::update()
{
    this->updateDataSource();
    this->updateActorTransform();
    this->updateActorProperties();

    if (m_modelIsVolume)
    {
        m_volume.GetPointer()->SetVisibility(m_visualModel->isVisible());
    }
    else
    {
        m_actor.GetPointer()->SetVisibility(m_visualModel->isVisible());
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
    if (this->isMesh())
    {
        updateActorPropertiesMesh();
        return;
    }
    if (this->isVolume())
    {
        updateActorPropertiesVolumeRendering();
        return;
    }

    // remove this because there should be a default visual model?
    // not visible: nothing to do; mind the initialization need the first time around
    if (!m_visualModel || !m_visualModel->isVisible())
    {
        return;
    }

    const auto material = m_visualModel->getRenderMaterial();
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
        
        switch (material->getShadingModel())
        {           
            case RenderMaterial::ShadingModel::Gouraud:
                actorProperty->SetInterpolationToGouraud();
                break;
            case RenderMaterial::ShadingModel::Flat:
                actorProperty->SetInterpolationToFlat();
                break;
            case RenderMaterial::ShadingModel::Phong:                               
            default:
                actorProperty->SetInterpolationToPhong();
        }
    }

    // Display properties
    actorProperty->SetLineWidth(material->m_lineWidth);
    actorProperty->SetPointSize(material->m_pointSize);
    actorProperty->SetBackfaceCulling(material->m_backfaceCulling);

    // Material state is now up to date
    material->m_stateModified = false;
}

void
VTKRenderDelegate::updateActorPropertiesMesh()
{
    // remove this because there should be a default visual model?
    // not visible: nothing to do; mind the initialization need the first time around
    if (!m_visualModel || !m_visualModel->isVisible())
    {
        return;
    }

    const auto material = m_visualModel->getRenderMaterial();
    if (!material || !material->isModified())
    {
        return;
    }

    const auto actorProperty = m_actor->GetProperty();

    // Material state is now up to date
    material->m_modified = false;
    if (!material->m_stateModified)
    {
        return;
    }
    
    const auto edgeColor = material->getEdgeColor();
    const auto vertexColor = material->getVertexColor();
    const auto surfaceColor = material->getColor();
    
    actorProperty->SetColor(surfaceColor.r, surfaceColor.g, surfaceColor.b);
    if (material->getDisplayMode() != RenderMaterial::DisplayMode::Surface)
    {
        switch (material->getDisplayMode())
        {
        case RenderMaterial::DisplayMode::Wireframe:
            actorProperty->SetRepresentationToWireframe();
            break;
        case RenderMaterial::DisplayMode::Points:
            actorProperty->SetRepresentationToPoints();
            break;
        default:
            actorProperty->SetRepresentationToSurface();//wireframeSurface
        }

        // enable vertex visibility and vertex edge properties
        actorProperty->SetEdgeVisibility(false);
        actorProperty->SetPointSize(material->getPointSize());
        actorProperty->SetRenderPointsAsSpheres(true);
        actorProperty->SetVertexVisibility(true);
        actorProperty->SetVertexColor(vertexColor.r, vertexColor.g, vertexColor.b);

        if (material->getDisplayMode() != RenderMaterial::DisplayMode::Points)
        {
            // enable edge visibility and set edge properties
            actorProperty->SetEdgeVisibility(true);
            actorProperty->SetEdgeColor(edgeColor.r, edgeColor.g, edgeColor.b);
            actorProperty->SetLineWidth(material->getLineWidth());
            actorProperty->SetRenderLinesAsTubes(true);
        }

        if (material->getDisplayMode() == RenderMaterial::DisplayMode::WireframeSurface)
        {
            switch (material->getShadingModel())
            {
                case RenderMaterial::ShadingModel::Flat:
                    actorProperty->SetInterpolationToFlat();
                    break;
                case RenderMaterial::ShadingModel::Gouraud:
                    actorProperty->SetInterpolationToGouraud();
                    break;
                default:
                    actorProperty->SetInterpolationToPhong();
            }
        }
    
    }
    else // surface
    {
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(false);
        actorProperty->SetVertexVisibility(false);
        if (material->getShadingModel() == RenderMaterial::ShadingModel::PBR)
        {
            
            /*actorProperty->UseImageBasedLightingOn();
            actorProperty->SetEnvironmentCubeMap(getVTKTexture(cubemap));*/
            
            actorProperty->SetInterpolationToPBR();

            // configure the basic properties
            actorProperty->SetColor(surfaceColor.r, surfaceColor.g, surfaceColor.b);
            actorProperty->SetMetallic(material->getMetalness());
            actorProperty->SetRoughness(material->getRoughness());

            // configure textures (needs tcoords on the mesh)
            const auto baseColorTexture = material->getTexture(Texture::Type::Diffuse);
            if (baseColorTexture)
            {
                actorProperty->SetBaseColorTexture(getVTKTexture(baseColorTexture));
            }

            const auto ormTexture = material->getTexture(Texture::Type::ORM);
            if (ormTexture)
            {
                actorProperty->SetORMTexture(getVTKTexture(ormTexture));
                actorProperty->SetOcclusionStrength(material->getOcclusionStrength());
            }

            const auto emmisiveTexture = material->getTexture(Texture::Type::Emissive);
            if (emmisiveTexture)
            {
                actorProperty->SetEmissiveTexture(getVTKTexture(emmisiveTexture));
                const double emmis = material->getEmissivity();
                actorProperty->SetEmissiveFactor(emmis, emmis, emmis);
            }            

            const auto normalTexture = material->getTexture(Texture::Type::Normal);
            if (normalTexture)
            {
                // needs tcoords, normals and tangents on the mesh
                actorProperty->SetNormalTexture(getVTKTexture(normalTexture));
                actorProperty->SetNormalScale(material->getNormalStrength());
            }
        }
        else if (material->getShadingModel() == RenderMaterial::ShadingModel::Phong)
        {
            actorProperty->SetInterpolationToPhong();
        }
        else if (material->getShadingModel() == RenderMaterial::ShadingModel::Gouraud)
        {
            actorProperty->SetInterpolationToGouraud();
        }
        else
        {
            actorProperty->SetInterpolationToFlat();
        }
    }

    // Display mode
    //switch (material->getDisplayMode())
    //{
    //case RenderMaterial::DisplayMode::Wireframe:
    //    actorProperty->SetRepresentationToWireframe();       
    //    actorProperty->SetEdgeVisibility(true);
    //    actorProperty->SetEdgeColor(ec.r, ec.g, ec.b);
    //    actorProperty->SetLineWidth(material->getLineWidth());
    //    actorProperty->SetPointSize(material->getPointSize());
    //    actorProperty->SetRenderLinesAsTubes(true);
    //    actorProperty->SetRenderPointsAsSpheres(true);
    //    actorProperty->SetVertexVisibility(true);
    //    actorProperty->SetVertexColor(vc.r, vc.g, vc.b);
    //    break;
    //case RenderMaterial::DisplayMode::Points:
    //    actorProperty->SetRepresentationToPoints();
    //    actorProperty->SetEdgeVisibility(false);              
    //    actorProperty->SetPointSize(material->getPointSize());        
    //    actorProperty->SetRenderPointsAsSpheres(true);
    //    actorProperty->SetVertexVisibility(true);
    //    actorProperty->SetVertexColor(vc.r, vc.g, vc.b);
    //    break;
    //case RenderMaterial::DisplayMode::WireframeSurface:
    //    actorProperty->SetRepresentationToSurface();
    //    actorProperty->SetColor(sc.r, sc.g, sc.b);
    //    actorProperty->SetEdgeVisibility(true);
    //    actorProperty->SetEdgeColor(ec.r, ec.g, ec.b);
    //    actorProperty->SetLineWidth(material->getLineWidth());
    //    actorProperty->SetPointSize(material->getPointSize());
    //    actorProperty->SetRenderLinesAsTubes(true);
    //    actorProperty->SetRenderPointsAsSpheres(true);
    //    actorProperty->SetVertexVisibility(true);
    //    actorProperty->SetVertexColor(vc.r, vc.g, vc.b);
    //    break;
    //case RenderMaterial::DisplayMode::Surface:
    //default:
    //    actorProperty->SetRepresentationToSurface();
    //    actorProperty->SetEdgeVisibility(false);
    //    if (material->getShadingModel() == RenderMaterial::ShadingModel::PBR)
    //    {
    //        //renderer->UseImageBasedLightingOn();
    //        //renderer->SetEnvironmentCubeMap(cubemap);
    //        actorProperty->SetInterpolationToPBR();
    //        // configure the basic properties
    //        actorProperty->SetColor(0.5, 1.0, 0.8);
    //        actorProperty->SetMetallic(material->getMetalness());
    //        actorProperty->SetRoughness(material->getRoughness());

    //        // configure textures (needs tcoords on the mesh)
    //        actorProperty->SetBaseColorTexture(getVTKTexture(material->getTexture(Texture::Type::Diffuse)));

    //        actorProperty->SetORMTexture(getVTKTexture(material->getTexture(Texture::Type::ORM)));
    //        actorProperty->SetOcclusionStrength(material->getOcclusionStrength());

    //        actorProperty->SetEmissiveTexture(getVTKTexture(material->getTexture(Texture::Type::Emissive)));
    //        const double emmis = material->getEmissivity();
    //        actorProperty->SetEmissiveFactor(emmis, emmis, emmis);

    //        // needs tcoords, normals and tangents on the mesh
    //        actorProperty->SetNormalTexture(getVTKTexture(material->getTexture(Texture::Type::Normal)));
    //        actorProperty->SetNormalScale(material->getNormalStrength());
    //    }
    //    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Phong)
    //    {
    //        actorProperty->SetInterpolationToPhong();
    //    }
    //    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Gouraud)
    //    {
    //        actorProperty->SetInterpolationToGouraud();
    //    }
    //    else
    //    {
    //        actorProperty->SetInterpolationToFlat();
    //    }
    //    break;
    //}

    actorProperty->SetBackfaceCulling(material->getBackfaceCulling());
    actorProperty->SetOpacity(material->getOpacity());

    // Material state is now up-to-date
    material->m_stateModified = false;
}

void
VTKRenderDelegate::updateActorPropertiesVolumeRendering()
{
    if (!m_visualModel || !m_visualModel->isVisible())
    {
        return;
    }

    const auto material = m_visualModel->getRenderMaterial();
    if (!material || !material->isModified())
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

    // Material state is now up to date
    material->setModified(false);

    // Material state is now up to date
    //material->m_stateModified = false;

}

vtkSmartPointer<vtkTexture>
VTKRenderDelegate::getVTKTexture(std::shared_ptr<Texture> texture)
{
    vtkNew<vtkImageReader2Factory> readerFactory;
    vtkSmartPointer<vtkImageReader2> imageReader;
    
    const std::string fileName = texture->getPath();

    imageReader.TakeReference(readerFactory->CreateImageReader2(fileName.c_str()));
    imageReader->SetFileName(fileName.c_str());
    imageReader->Update();

    // Create texture
    vtkNew<vtkTexture> vtktexture;
    vtktexture->SetInputConnection(imageReader->GetOutputPort());

    return vtktexture;
}


std::shared_ptr<VisualModel> 
VTKRenderDelegate::getVisualModel() const
{
    return m_visualModel;
}

} // imstk
