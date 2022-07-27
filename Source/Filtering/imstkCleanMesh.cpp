/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCleanMesh.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkLogger.h"
#include "imstkGeometryUtilities.h"

#include <vtkCleanPolyData.h>

namespace imstk
{
CleanMesh::CleanMesh()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

std::shared_ptr<SurfaceMesh>
CleanMesh::getOutputMesh() const
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
CleanMesh::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

void
CleanMesh::requestUpdate()
{
    std::shared_ptr<PointSet> inputMesh = std::dynamic_pointer_cast<PointSet>(getInput(0));
    if (inputMesh == nullptr)
    {
        LOG(WARNING) << "No inputMesh to clean";
        return;
    }
    vtkSmartPointer<vtkPolyData> inputMeshVtk = nullptr;
    if (auto lineMesh = std::dynamic_pointer_cast<LineMesh>(inputMesh))
    {
        inputMeshVtk = GeometryUtils::copyToVtkPolyData(lineMesh);
    }
    else if (auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(inputMesh))
    {
        inputMeshVtk = GeometryUtils::copyToVtkPolyData(surfMesh);
    }
    else
    {
        LOG(WARNING) << "Unsupported mesh type";
        return;
    }

    vtkNew<vtkCleanPolyData> filter;
    filter->SetInputData(inputMeshVtk);
    filter->SetTolerance(m_Tolerance);
    filter->SetAbsoluteTolerance(m_AbsoluteTolerance);
    filter->SetToleranceIsAbsolute(m_UseAbsolute);
    filter->Update();

    if (std::dynamic_pointer_cast<LineMesh>(inputMesh) != nullptr)
    {
        setOutput(GeometryUtils::copyToLineMesh(filter->GetOutput()));
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(inputMesh) != nullptr)
    {
        setOutput(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
    }
}
} // namespace imstk