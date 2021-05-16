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

#include "imstkImageResample.h"
#include "imstkDataArray.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkParallelFor.h"
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkSmartPointer.h>

namespace imstk
{
ImageResample::ImageResample()
{
    setInputPortReq<ImageData>(0);

    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
    setOutput(std::make_shared<ImageData>());
}

std::shared_ptr<ImageData>
ImageResample::getOutputImage() const
{
    return std::dynamic_pointer_cast<ImageData>(getOutput(0));
}

void
ImageResample::setInputImage(std::shared_ptr<ImageData> inputData)
{
    setInput(inputData, 0);
}

void
ImageResample::requestUpdate()
{
    std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
    if (inputImage == nullptr)
    {
        LOG(WARNING) << "No inputImage to resample";
        return;
    }

    vtkNew<vtkImageResample> resample;
    resample->SetInputData(GeometryUtils::copyToVtkImageData(inputImage));
    resample->SetInterpolationModeToLinear();
    resample->SetOutputExtent(0, m_Dimensions[0] - 1, 0, m_Dimensions[1] - 1, 0, m_Dimensions[2] - 1);
    resample->Update();

    setOutput(GeometryUtils::copyToImageData(resample->GetOutput()));
}
}