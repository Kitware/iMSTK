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

#include "imstkImageReslice.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"

#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkTransform.h>

namespace imstk
{
ImageReslice::ImageReslice()
{
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
    setOutput(std::make_shared<ImageData>());
}

std::shared_ptr<ImageData>
ImageReslice::getOutputImage() const
{
    return std::dynamic_pointer_cast<ImageData>(getOutput(0));
}

void
ImageReslice::setInputImage(std::shared_ptr<ImageData> inputData)
{
    setInput(inputData, 0);
}

void
ImageReslice::requestUpdate()
{
    std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
    if (inputImage == nullptr)
    {
        LOG(WARNING) << "No inputImage to resample";
        return;
    }

    vtkNew<vtkTransform> transform;
    Mat4d                test = m_Transform.transpose();
    transform->SetMatrix(test.data());

    vtkNew<vtkImageReslice> reslice;
    reslice->SetResliceTransform(transform);
    if (m_InterpolationType == InterpolateType::NearestNeighbor)
    {
        reslice->SetInterpolationModeToNearestNeighbor();
    }
    else if (m_InterpolationType == InterpolateType::Linear)
    {
        reslice->SetInterpolationModeToLinear();
    }
    else if (m_InterpolationType == InterpolateType::Cubic)
    {
        reslice->SetInterpolationModeToCubic();
    }
    vtkSmartPointer<vtkImageData> vtkInputImage = GeometryUtils::copyToVtkImageData(inputImage);
    reslice->SetInputData(vtkInputImage);
    reslice->SetResliceTransform(transform);
    reslice->SetAutoCropOutput(true);
    reslice->Update();

    setOutput(GeometryUtils::copyToImageData(reslice->GetOutput()));
}
}