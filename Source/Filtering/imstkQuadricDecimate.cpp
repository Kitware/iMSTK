/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkQuadricDecimate.h"
#include "imstkSurfaceMesh.h"
#include "imstkLogger.h"
#include "imstkGeometryUtilities.h"

#include <vtkQuadricDecimation.h>

namespace imstk
{
QuadricDecimate::QuadricDecimate() :
    m_VolumePreserving(true), m_TargetReduction(0.6)
{
    setRequiredInputType<SurfaceMesh>(0);

    setNumInputPorts(1);
    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
QuadricDecimate::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

void
QuadricDecimate::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    if (inputMesh == nullptr)
    {
        LOG(WARNING) << "No inputMesh to clean";
        return;
    }
    vtkSmartPointer<vtkPolyData> inputMeshVtk = GeometryUtils::copyToVtkPolyData(std::dynamic_pointer_cast<SurfaceMesh>(inputMesh));

    vtkNew<vtkQuadricDecimation> filter;
    filter->SetInputData(inputMeshVtk);
    filter->SetVolumePreservation(m_VolumePreserving);
    filter->SetTargetReduction(m_TargetReduction);
    filter->Update();

    setOutput(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
}
} // namespace imstk