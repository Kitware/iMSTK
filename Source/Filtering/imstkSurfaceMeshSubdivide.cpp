/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshSubdivide.h"
#include "imstkGeometryUtilities.h"
#include "imstkSurfaceMesh.h"
#include "imstkLogger.h"

#include <vtkButterflySubdivisionFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>

namespace imstk
{
SurfaceMeshSubdivide::SurfaceMeshSubdivide()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

std::shared_ptr<SurfaceMesh>
SurfaceMeshSubdivide::getOutputMesh()
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
SurfaceMeshSubdivide::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

void
SurfaceMeshSubdivide::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));

    if (inputMesh == nullptr)
    {
        LOG(WARNING) << "Missing required SurfaceMesh input";
        return;
    }

    vtkSmartPointer<vtkSubdivisionFilter> filter = nullptr;
    if (m_SubdivisionType == Type::BUTTERFLY)
    {
        filter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
    }
    else if (m_SubdivisionType == Type::LOOP)
    {
        filter = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
    }
    else
    {
        filter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
    }

    filter->SetInputData(GeometryUtils::copyToVtkPolyData(inputMesh));
    filter->SetNumberOfSubdivisions(m_NumberOfSubdivisions);
    filter->Update();

    setOutput(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
}
} // namespace imstk