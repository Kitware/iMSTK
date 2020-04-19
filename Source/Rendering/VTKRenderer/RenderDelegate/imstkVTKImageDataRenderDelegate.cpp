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
#include "imstkImageData.h"

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkTrivialProducer.h>
#include <vtkVolumeProperty.h>
#include <vtkImageData.h>

namespace imstk
{
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    m_visualModel = visualModel;

    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());
    if (imageData->getData())
    {
        auto tp = vtkSmartPointer<vtkTrivialProducer>::New();
        tp->SetOutput(imageData->getData());
        this->setUpMapper(tp->GetOutputPort(), false,
                          m_visualModel->getRenderMaterial());
    }
}

void
VTKImageDataRenderDelegate::updateDataSource()
{
    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());

    if (!imageData->m_dataModified)
    {
        return;
    }

    // TODO: Any transforms/modifications?
    imageData->m_dataModified = false;
}
} // imstk
