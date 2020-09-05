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
#include "imstkVisualModel.h"
#include "imstkDataArray.h"

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkTrivialProducer.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

namespace imstk
{
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    m_visualModel   = visualModel;
    m_isMesh        = false;
    m_modelIsVolume = true;

    auto imageData = std::dynamic_pointer_cast<ImageData>(m_visualModel->getGeometry());
    m_scalarArray = imageData->getScalars();

    // Couple the imstkImageData with vtkImageData
    imageDataVtk = GeometryUtils::coupleVtkImageData(imageData);

    vtkNew<vtkTrivialProducer> tp;
    tp->SetOutput(imageDataVtk);
    this->setUpMapper(tp->GetOutputPort(), m_visualModel);

    this->updateActorPropertiesVolumeRendering(); // check if this is needed once more!

    // When image is modified, queue an event to this object that calls imageDataModified
    queueConnect<Event>(imageData, EventType::Modified, this, &VTKImageDataRenderDelegate::imageDataModified);
}

void
VTKImageDataRenderDelegate::updateDataSource()
{
    // Handle any queued events
    // todo: Eventually move this to renderdelegate base class
    doLastEvent();
}

void
VTKImageDataRenderDelegate::imageDataModified(Event* imstkNotUsed(e))
{
    // Here we utilize something similar to double buffering
    // We primarily use a single shared buffer until the moment
    // it is reallocated
    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());

    // If our handle is not up to date, update it
    if (m_scalarArray != imageData->getScalars())
    {
        // Update our handle
        m_scalarArray = imageData->getScalars();

        // Update vtk data array pointer to this new handle, this will cause existing array to deallocate assuming no
        // one else is referencing it
        imageDataVtk->GetPointData()->GetScalars()->SetVoidArray(m_scalarArray->getVoidPointer(), m_scalarArray->size(), 1);

        // Update information (currently can't handle type changes or number of components)
        const Vec3i& dim = imageData->getDimensions();
        imageDataVtk->SetDimensions(dim.data());
        imageDataVtk->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
        const Vec3d vtkOrigin = imageData->getOrigin() + imageData->getSpacing() * 0.5;
        imageDataVtk->SetOrigin(vtkOrigin.data());
        imageDataVtk->SetSpacing(imageData->getSpacing().data());
    }

    // Always post modified on VTK data, when modified is recieved
    m_volumeMapper->GetInput()->Modified();

    // \todo: transforms
    if (imageData->m_transformModified)
    {
        imageData->m_transformModified = false;
    }
}
} // imstk
