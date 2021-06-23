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
    setNumberOfInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumberOfOutputPorts(1);
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
}