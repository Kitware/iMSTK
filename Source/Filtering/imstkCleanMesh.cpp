/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

          http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
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
}