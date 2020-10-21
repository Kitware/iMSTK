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

#include "imstkImageGradient.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"

#include <vtkImageData.h>
#include <vtkImageGradient.h>
#include <vtkImageGradientMagnitude.h>

namespace imstk
{
ImageGradient::ImageGradient()
{
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
    setOutput(std::make_shared<PointSet>());
}

void
ImageGradient::setInputImage(std::shared_ptr<ImageData> inputImage)
{
    setInput(inputImage, 0);
}

void
ImageGradient::requestUpdate()
{
    std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
    if (inputImage == nullptr)
    {
        LOG(WARNING) << "No input to compute gradients";
        return;
    }
    if (inputImage->getNumComponents() != 1)
    {
        LOG(WARNING) << "Can only compute gradient on single channel image";
        return;
    }
    vtkSmartPointer<vtkImageData> inputImageVtk = GeometryUtils::coupleVtkImageData(inputImage);
    if (m_ComputeMagnitude)
    {
        vtkNew<vtkImageGradientMagnitude> gradientMagnitude;
        gradientMagnitude->SetInputData(inputImageVtk);
        gradientMagnitude->Update();
        setOutput(GeometryUtils::copyToImageData(gradientMagnitude->GetOutput()));
    }
    else
    {
        vtkNew<vtkImageGradient> gradients;
        gradients->SetInputData(inputImageVtk);
        gradients->SetHandleBoundaries(true);
        gradients->Update();
        setOutput(GeometryUtils::copyToImageData(gradients->GetOutput()));
    }
}
}