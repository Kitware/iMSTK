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

#include <vtkImageData.h>
#include <vtkTrivialProducer.h>

namespace imstk
{
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    m_visualModel   = visualModel;
    m_isMesh        = false;
    m_modelIsVolume = true;

    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());
    if (imageData->getScalars() != nullptr)
    {
        vtkSmartPointer<vtkImageData> imageDataVtk = GeometryUtils::coupleVtkImageData(imageData);
        auto                          tp = vtkSmartPointer<vtkTrivialProducer>::New();
        tp->SetOutput(imageDataVtk);
        this->setUpMapper(tp->GetOutputPort(), m_visualModel);

        this->updateActorPropertiesVolumeRendering();// check if this is needed once more!
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
