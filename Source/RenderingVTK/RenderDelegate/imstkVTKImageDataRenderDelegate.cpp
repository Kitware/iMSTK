/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate() :
    m_scalarArray(nullptr),
    m_imageDataVtk(nullptr)
{
}

void
VTKImageDataRenderDelegate::init()
{
    auto imageData = std::dynamic_pointer_cast<ImageData>(m_visualModel->getGeometry());
    CHECK(imageData != nullptr) << "VTKImageDataRenderDelegate only works with ImageData geometry";
    m_scalarArray = imageData->getScalars();

    // Couple the imstkImageData with vtkImageData
    m_imageDataVtk = GeometryUtils::coupleVtkImageData(imageData);

    // When the image is modified
    queueConnect<Event>(imageData, &ImageData::modified,
        std::static_pointer_cast<VTKImageDataRenderDelegate>(shared_from_this()),
        &VTKImageDataRenderDelegate::imageDataModified);

    // When the image scalars are modified
    queueConnect<Event>(imageData->getScalars(), &AbstractDataArray::modified,
        std::static_pointer_cast<VTKImageDataRenderDelegate>(shared_from_this()),
        &VTKImageDataRenderDelegate::imageScalarsModified);

    // Setup mapper
    {
        vtkNew<vtkGPUVolumeRayCastMapper> mapper;
        mapper->SetInputData(m_imageDataVtk);
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
        m_imageDataVtk->GetPointData()->GetScalars()->SetVoidArray(m_scalarArray->getVoidPointer(), m_scalarArray->size(), 1);

        // Update information
        // \todo: Can't handle type changes or number of component changes
        const Vec3i& dim = imageData->getDimensions();
        m_imageDataVtk->SetDimensions(dim.data());
        m_imageDataVtk->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
        const Vec3d vtkOrigin = imageData->getOrigin() + imageData->getSpacing() * 0.5;
        m_imageDataVtk->SetOrigin(vtkOrigin.data());
        m_imageDataVtk->SetSpacing(imageData->getSpacing().data());
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
    if (m_scalarArray->getVoidPointer() != m_imageDataVtk->GetPointData()->GetScalars()->GetVoidPointer(0))
    {
        m_imageDataVtk->GetPointData()->GetScalars()->SetVoidArray(m_scalarArray->getVoidPointer(), m_scalarArray->size(), 1);
    }
    volumeMapper->GetInput()->Modified();
}
} // namespace imstk