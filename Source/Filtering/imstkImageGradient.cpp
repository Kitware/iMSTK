/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    setNumInputPorts(1);
    setRequiredInputType<ImageData>(0);

    setNumOutputPorts(1);
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
} // namespace imstk