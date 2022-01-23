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