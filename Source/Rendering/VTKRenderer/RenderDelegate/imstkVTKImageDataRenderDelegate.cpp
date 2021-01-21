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

#include "imstkVTKImageDataRenderDelegate.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkVolume.h>
#include <vtkTransform.h>

namespace imstk
{
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKVolumeRenderDelegate(visualModel),
    m_scalarArray(nullptr),
    imageDataVtk(nullptr)
{
    auto imageData = std::dynamic_pointer_cast<ImageData>(m_visualModel->getGeometry());
    m_scalarArray = imageData->getScalars();

    // Couple the imstkImageData with vtkImageData
    imageDataVtk = GeometryUtils::coupleVtkImageData(imageData);

    // When the image is modified
    queueConnect<Event>(imageData, EventType::Modified, this, &VTKImageDataRenderDelegate::imageDataModified);

    // When the image scalars are modified
    queueConnect<Event>(imageData->getScalars(), EventType::Modified, this, &VTKImageDataRenderDelegate::imageScalarsModified);

    // Setup mapper
    {
        vtkNew<vtkGPUVolumeRayCastMapper> mapper;
        mapper->SetInputData(imageDataVtk);
        vtkNew<vtkVolume> volume;
        volume->SetMapper(mapper);
        volume->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = volume;
    }

    update();
    updateRenderProperties();
}

void
VTKImageDataRenderDelegate::processEvents()
{
    // This handler chooses and executes the latest event from each respective sender
    std::shared_ptr<ImageData> geom = std::dynamic_pointer_cast<ImageData>(m_visualModel->getGeometry());

    // Only use the most recent event from respective sender
    std::list<Command> cmds;
    bool               contains[4] = { false, false, false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds.push_back(cmd);
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == m_material.get() && !contains[1])
            {
                cmds.push_back(cmd);
                contains[1] = true;
            }
            else if (cmd.m_event->m_sender == geom.get() && !contains[2])
            {
                cmds.push_back(cmd);
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == geom->getScalars().get() && !contains[3])
            {
                cmds.push_back(cmd);
                contains[3] = true;
            }
        });
    // Now do each event in order recieved
    for (std::list<Command>::reverse_iterator i = cmds.rbegin(); i != cmds.rend(); i++)
    {
        i->invoke();
    }
}

void
VTKImageDataRenderDelegate::imageDataModified(Event* imstkNotUsed(e))
{
    auto                                       imageData    = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(m_mapper);

    // If the user swapped scalars on us
    if (m_scalarArray != imageData->getScalars())
    {
        // Update our handle
        m_scalarArray = imageData->getScalars();

        // Update vtk data array pointer
        imageDataVtk->GetPointData()->GetScalars()->SetVoidArray(m_scalarArray->getVoidPointer(), m_scalarArray->size(), 1);

        // Update information
        // \todo: Can't handle type changes or number of component changes
        const Vec3i& dim = imageData->getDimensions();
        imageDataVtk->SetDimensions(dim.data());
        imageDataVtk->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
        const Vec3d vtkOrigin = imageData->getOrigin() + imageData->getSpacing() * 0.5;
        imageDataVtk->SetOrigin(vtkOrigin.data());
        imageDataVtk->SetSpacing(imageData->getSpacing().data());
    }
    volumeMapper->GetInput()->Modified();
}

void
VTKImageDataRenderDelegate::imageScalarsModified(Event* imstkNotUsed(e))
{
    auto                                       geometry     = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(m_mapper);
    m_scalarArray = geometry->getScalars();

    // If pointer changed, update the one vtk is viewing
    if (m_scalarArray->getVoidPointer() != imageDataVtk->GetPointData()->GetScalars()->GetVoidPointer(0))
    {
        imageDataVtk->GetPointData()->GetScalars()->SetVoidArray(m_scalarArray->getVoidPointer(), m_scalarArray->size(), 1);
    }
    volumeMapper->GetInput()->Modified();
}
} // imstk
