/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshSmoothen.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkSmoothPolyDataFilter.h>

namespace imstk
{
SurfaceMeshSmoothen::SurfaceMeshSmoothen()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
SurfaceMeshSmoothen::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

void
SurfaceMeshSmoothen::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    if (inputMesh == nullptr)
    {
        LOG(WARNING) << "No inputMesh to smoothen";
        return;
    }

    vtkNew<vtkSmoothPolyDataFilter> filter;
    filter->SetInputData(GeometryUtils::copyToVtkPolyData(inputMesh));
    filter->SetNumberOfIterations(m_NumberOfIterations);
    filter->SetRelaxationFactor(m_RelaxationFactor);
    filter->SetConvergence(m_Convergence);
    filter->SetFeatureAngle(m_FeatureAngle);
    filter->SetEdgeAngle(m_EdgeAngle);
    filter->SetFeatureEdgeSmoothing(m_FeatureEdgeSmoothing);
    filter->SetBoundarySmoothing(m_BoundarySmoothing);
    filter->Update();

    setOutput(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
}
} // namespace imstk