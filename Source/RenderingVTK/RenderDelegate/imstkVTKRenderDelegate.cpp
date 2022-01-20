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
#include "imstkPointSet.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"

// VTK render delegates
#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkVTKFluidRenderDelegate.h"
#include "imstkVTKHexahedralMeshRenderDelegate.h"
#include "imstkVTKImageDataRenderDelegate.h"
#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkVTKOrientedBoxRenderDelegate.h"
#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkVTKPointSetRenderDelegate.h"
#include "imstkVTKSphereRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKSurfaceNormalRenderDelegate.h"
#include "imstkVTKTetrahedralMeshRenderDelegate.h"

#include <vtkActor.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTexture.h>
#include <vtkTransform.h>

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
    queueConnect<Event>(m_material, &RenderMaterial::modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::materialModified);

    // When the visual model is modified call visualModelModified
    queueConnect<Event>(m_visualModel, &VisualModel::modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::visualModelModified);
}

std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::makeDelegate(std::shared_ptr<VisualModel> visualModel)
{
    const std::string geomType = visualModel->getGeometry()->getTypeName();

    // Two edge cases
    if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
    {
        if (std::dynamic_pointer_cast<PointSet>(visualModel->getGeometry()) != nullptr)
        {
            return std::make_shared<VTKFluidRenderDelegate>(visualModel);
        }
    }
    if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::SurfaceNormals)
    {
        if (std::dynamic_pointer_cast<PointSet>(visualModel->getGeometry()) != nullptr)
        {
            return std::make_shared<VTKSurfaceNormalRenderDelegate>(visualModel);
        }
    }

    if (visualModel->getGeometry()->isMesh())
    {
        if (geomType == "SurfaceMesh")
        {
            return std::make_shared<VTKSurfaceMeshRenderDelegate>(visualModel);
        }
        else if (geomType == "TetrahedralMesh")
        {
            return std::make_shared<VTKTetrahedralMeshRenderDelegate>(visualModel);
        }
        else if (geomType == "LineMesh")
        {
            return std::make_shared<VTKLineMeshRenderDelegate>(visualModel);
        }
        else if (geomType == "HexahedralMesh")
        {
            return std::make_shared<VTKHexahedralMeshRenderDelegate>(visualModel);
        }
    }
    else
    {
        if (geomType == "PointSet")
        {
            return std::make_shared<VTKPointSetRenderDelegate>(visualModel);
        }
        else if (geomType == "Plane")
        {
            return std::make_shared<VTKPlaneRenderDelegate>(visualModel);
        }
        else if (geomType == "Sphere")
        {
            return std::make_shared<VTKSphereRenderDelegate>(visualModel);
        }
        else if (geomType == "Capsule")
        {
            return std::make_shared<VTKCapsuleRenderDelegate>(visualModel);
        }
        else if (geomType == "OrientedBox")
        {
            return std::make_shared<VTKOrientedCubeRenderDelegate>(visualModel);
        }
        else if (geomType == "Cylinder")
        {
            return std::make_shared<VTKCylinderRenderDelegate>(visualModel);
        }
        else if (geomType == "ImageData")
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
    }
    LOG(FATAL) << "RenderDelegate::makeDelegate error: Geometry type incorrect.";
    return nullptr;
}

void
VTKRenderDelegate::update()
{
    // Then leave it up to subclasses to implement how to process the events
    processEvents();
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
    disconnect(m_material, this, &RenderMaterial::modified);

    m_material = m_visualModel->getRenderMaterial(); // Update handle

    // Recieve events from new material
    queueConnect<Event>(m_material, &RenderMaterial::modified, static_cast<VTKRenderDelegate*>(this), &VTKRenderDelegate::materialModified);

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
}