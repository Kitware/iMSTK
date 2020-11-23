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
#include "imstkDebugRenderGeometry.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"

// Debug render delegates
#include "imstkVTKdebugLinesRenderDelegate.h"
#include "imstkVTKdebugPointsRenderDelegate.h"
#include "imstkVTKdebugTrianglesRenderDelegate.h"
#include "imstkColorFunction.h"

// VTK render delegates
#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkVTKCubeRenderDelegate.h"
#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkVTKFluidRenderDelegate.h"
#include "imstkVTKHexahedralMeshRenderDelegate.h"
#include "imstkVTKImageDataRenderDelegate.h"
#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkVTKPointSetRenderDelegate.h"
#include "imstkVTKSphereRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKTetrahedralMeshRenderDelegate.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

namespace imstk
{
VTKRenderDelegate::VTKRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_transform(vtkSmartPointer<vtkTransform>::New()),
    m_actor(nullptr),
    m_mapper(nullptr),
    m_visualModel(visualModel),
    m_material(visualModel->getRenderMaterial())
{
    // When render material is modified call materialModified -> updateRenderProperties()
    queueConnect<Event>(m_material, EventType::Modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::materialModified);

    // When the visual model is modified call visualModelModified
    queueConnect<Event>(m_visualModel, EventType::Modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::visualModelModified);
}

std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::makeDelegate(std::shared_ptr<VisualModel> visualModel)
{
    if (visualModel->getGeometry()->isMesh())
    {
        if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
        {
            return std::make_shared<VTKFluidRenderDelegate>(visualModel);
        }

        switch (visualModel->getGeometry()->getType())
        {
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
        default:
        {
            LOG(FATAL) << "RenderDelegate::makeDelegate error: Mesh type incorrect.";
            return nullptr;     // will never be reached
        }
        }
    }
    else
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
        case Geometry::Type::ImageData:
        {
            if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Points)
            {
                return std::make_shared<VTKPointSetRenderDelegate>(visualModel);
            }
            else
            {
                return std::make_shared<VTKImageDataRenderDelegate>(visualModel);
            }
        }
        default:
        {
            LOG(FATAL) << "RenderDelegate::makeDelegate error: Geometry type incorrect.";
            return nullptr;     // will never be reached
        }
        }
    }
}

std::shared_ptr<VTKRenderDelegate>
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
        LOG(FATAL) << "RenderDelegate::makeDebugDelegate error: Geometry type incorrect.";
        return nullptr; // will never be reached
    }
    }
    return nullptr;
}

void
VTKRenderDelegate::update()
{
    // Always update the actor transform
    updateActorTransform();

    // Then leave it up to subclasses to implement how to process the events
    processEvents();
}

void
VTKRenderDelegate::updateActorTransform()
{
    auto              geom = m_visualModel->getGeometry();
    AffineTransform3d T(geom->m_transform.matrix());
    T.scale(geom->getScaling());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
    m_transform->Modified();
}

void
VTKRenderDelegate::processEvents()
{
    std::shared_ptr<RenderMaterial> renderMaterial = m_visualModel->getRenderMaterial();

    // Only use the most recent event from respective sender
    std::list<Command> cmds;
    bool               contains[2] = { false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds.push_back(cmd);
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == renderMaterial.get() && !contains[1])
            {
                cmds.push_back(cmd);
                contains[1] = true;
            }
        });
    // Now do each event in order recieved
    for (std::list<Command>::reverse_iterator i = cmds.rbegin(); i != cmds.rend(); i++)
    {
        i->invoke();
    }
}

void
VTKRenderDelegate::visualModelModified(Event* imstkNotUsed(e))
{
    // Remove all modified's from the old material
    disconnect(m_material, this, EventType::Modified);

    m_material = m_visualModel->getRenderMaterial(); // Update handle

    // Recieve events from new material
    queueConnect<Event>(m_material, EventType::Modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::materialModified);

    // Update our render properties
    updateRenderProperties();
}

vtkSmartPointer<vtkTexture>
VTKRenderDelegate::getVTKTexture(std::shared_ptr<Texture> texture)
{
    vtkNew<vtkImageReader2Factory> readerFactory;
    std::string                    fileName    = texture->getPath();
    auto                           imageReader = readerFactory->CreateImageReader2(fileName.c_str());

    //imageReader.TakeReference(readerFactory->CreateImageReader2(fileName.c_str()));
    imageReader->SetFileName(fileName.c_str());
    imageReader->Update();

    // Create texture
    vtkNew<vtkTexture> vtktexture;
    //vtktexture->UseSRGBColorSpaceOn();
    vtktexture->SetInputConnection(imageReader->GetOutputPort());

    return vtktexture;
}
} // imstk
