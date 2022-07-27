/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    setNumInputPorts(1);
    setRequiredInputType<ImageData>(0);

    setNumOutputPorts(1);
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
} // namespace imstk