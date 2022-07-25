/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkExtractEdges.h"
#include "imstkGeometryUtilities.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkExtractEdges.h>
#include <vtkTriangleFilter.h>

namespace imstk
{
ExtractEdges::ExtractEdges()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<LineMesh>());
}

void
ExtractEdges::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

std::shared_ptr<LineMesh>
ExtractEdges::getOutputMesh() const
{
    return std::static_pointer_cast<LineMesh>(getOutput(0));
}

void
ExtractEdges::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    if (inputMesh == nullptr)
    {
        LOG(WARNING) << "No inputMesh to extract edges from";
        return;
    }

    vtkNew<vtkExtractEdges> extractEdges;
    extractEdges->SetInputData(GeometryUtils::copyToVtkPolyData(inputMesh));
    extractEdges->Update();

    vtkNew<vtkTriangleFilter> triangleFilter;
    triangleFilter->SetInputData(extractEdges->GetOutput());
    triangleFilter->Update();

    setOutput(GeometryUtils::copyToLineMesh(triangleFilter->GetOutput()));
}
} // namespace imstk