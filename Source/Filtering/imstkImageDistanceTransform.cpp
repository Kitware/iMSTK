/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkImageDistanceTransform.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"

#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageEuclideanDistance.h>
#include <vtkImageMathematics.h>
#include <vtkImageShiftScale.h>

namespace imstk
{
ImageDistanceTransform::ImageDistanceTransform()
{
    setNumInputPorts(1);
    setRequiredInputType<ImageData>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<ImageData>(), 0);
}

std::shared_ptr<ImageData>
ImageDistanceTransform::getOutputImage() const
{
    return std::dynamic_pointer_cast<ImageData>(getOutput(0));
}

void
ImageDistanceTransform::setInputImage(std::shared_ptr<ImageData> refImage)
{
    setInput(refImage, 0);
}

void
ImageDistanceTransform::requestUpdate()
{
    std::shared_ptr<ImageData> imageInput = std::dynamic_pointer_cast<ImageData>(getInput(0));

    if (imageInput == nullptr)
    {
        LOG(WARNING) << "Missing input image data";
        return;
    }

    vtkSmartPointer<vtkImageData> imageInputVtk = GeometryUtils::coupleVtkImageData(imageInput);

    // Compute the inner distance
    vtkSmartPointer<vtkImageEuclideanDistance> innerDistanceFilter = vtkSmartPointer<vtkImageEuclideanDistance>::New();
    innerDistanceFilter->SetInputData(imageInputVtk);
    innerDistanceFilter->ConsiderAnisotropyOn();
    innerDistanceFilter->SetAlgorithmToSaito();
    innerDistanceFilter->SetDimensionality(3);
    innerDistanceFilter->Update();
    vtkSmartPointer<vtkImageCast> innerCastFilter = vtkSmartPointer<vtkImageCast>::New();
    innerCastFilter->SetInputData(innerDistanceFilter->GetOutput());
    innerCastFilter->SetOutputScalarTypeToFloat();
    innerCastFilter->Update();
    vtkSmartPointer<vtkImageMathematics> innerSqrtFilter = vtkSmartPointer<vtkImageMathematics>::New();
    innerSqrtFilter->SetInput1Data(innerCastFilter->GetOutput());
    innerSqrtFilter->SetOperationToSquareRoot();
    innerSqrtFilter->Update();

    // Invert the input mask to compute the outer distance transform
    double*                             range = imageInputVtk->GetScalarRange();
    vtkSmartPointer<vtkImageShiftScale> invertFilter = vtkSmartPointer<vtkImageShiftScale>::New();
    invertFilter->SetInputData(imageInputVtk);
    invertFilter->SetShift(-range[1]);
    invertFilter->SetScale(-1.0);
    invertFilter->Update();

    // Compute the outer distance
    vtkSmartPointer<vtkImageEuclideanDistance> outerDistanceFilter = vtkSmartPointer<vtkImageEuclideanDistance>::New();
    outerDistanceFilter->SetInputData(invertFilter->GetOutput());
    outerDistanceFilter->ConsiderAnisotropyOn();
    outerDistanceFilter->SetAlgorithmToSaito();
    outerDistanceFilter->SetDimensionality(3);
    outerDistanceFilter->Update();
    vtkSmartPointer<vtkImageCast> outerCastFilter = vtkSmartPointer<vtkImageCast>::New();
    outerCastFilter->SetInputData(outerDistanceFilter->GetOutput());
    outerCastFilter->SetOutputScalarTypeToFloat();
    outerCastFilter->Update();
    vtkSmartPointer<vtkImageMathematics> outerSqrtFilter = vtkSmartPointer<vtkImageMathematics>::New();
    outerSqrtFilter->SetInput1Data(outerCastFilter->GetOutput());
    outerSqrtFilter->SetOperationToSquareRoot();
    outerSqrtFilter->Update();

    // Inner distance should be negative
    vtkSmartPointer<vtkImageData> innerImage = innerSqrtFilter->GetOutput();
    if (!m_UseUnsigned)
    {
        vtkSmartPointer<vtkImageShiftScale> negFilter = vtkSmartPointer<vtkImageShiftScale>::New();
        negFilter->SetInputData(innerSqrtFilter->GetOutput());
        negFilter->SetScale(-1.0);
        negFilter->Update();
        innerImage = negFilter->GetOutput();
    }

    // Finally combine the negative and positive
    vtkSmartPointer<vtkImageMathematics> addImageFilter = vtkSmartPointer<vtkImageMathematics>::New();
    addImageFilter->SetInput1Data(outerSqrtFilter->GetOutput());
    addImageFilter->SetInput2Data(innerImage);
    addImageFilter->SetOperationToAdd();
    addImageFilter->Update();

    setOutput(GeometryUtils::copyToImageData(addImageFilter->GetOutput()));
}
} // namespace imstk