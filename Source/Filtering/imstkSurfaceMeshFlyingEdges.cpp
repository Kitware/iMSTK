/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkFlyingEdges3D.h>
#include <vtkImageData.h>

namespace imstk
{
SurfaceMeshFlyingEdges::SurfaceMeshFlyingEdges()
{
    setNumInputPorts(1);
    setRequiredInputType<ImageData>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
SurfaceMeshFlyingEdges::setInputImage(std::shared_ptr<ImageData> inputImage)
{
    setInput(inputImage, 0);
}

std::shared_ptr<SurfaceMesh>
SurfaceMeshFlyingEdges::getOutputMesh() const
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
SurfaceMeshFlyingEdges::requestUpdate()
{
    std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
    if (inputImage == nullptr)
    {
        LOG(WARNING) << "No inputImage to extract isosurface from";
        return;
    }

    vtkNew<vtkFlyingEdges3D> filter;
    filter->SetInputData(GeometryUtils::coupleVtkImageData(inputImage));
    filter->SetValue(0, m_IsoValue);
    filter->ComputeNormalsOff();
    filter->ComputeScalarsOff();
    filter->ComputeGradientsOff();
    filter->Update();

    std::shared_ptr<SurfaceMesh> outputSurfMesh = GeometryUtils::copyToSurfaceMesh(filter->GetOutput());
    *std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0)) = *outputSurfMesh->clone();
}
} // namespace imstk