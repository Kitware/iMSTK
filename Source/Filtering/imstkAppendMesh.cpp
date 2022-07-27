/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAppendMesh.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkAppendPolyData.h>

namespace imstk
{
AppendMesh::AppendMesh()
{
    setNumInputPorts(1);
    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
AppendMesh::addInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setNumInputPorts(getNumInputPorts() + 1);
    setInput(inputMesh, getNumInputPorts() - 1);
}

std::shared_ptr<SurfaceMesh>
AppendMesh::getOutputMesh() const
{
    return std::static_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
AppendMesh::requestUpdate()
{
    vtkNew<vtkAppendPolyData> filter;
    for (size_t i = 0; i < getNumInputPorts(); i++)
    {
        std::shared_ptr<SurfaceMesh> inputMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
        if (inputMesh == nullptr)
        {
            LOG(WARNING) << "Input " << i << " invalid";
            return;
        }
        filter->AddInputData(GeometryUtils::copyToVtkPolyData(inputMesh));
    }
    filter->Update();

    setOutput(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
}
} // namespace imstk