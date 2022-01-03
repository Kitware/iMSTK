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

#include "imstkAppendMesh.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkAppendPolyData.h>

namespace imstk
{
AppendMesh::AppendMesh()
{
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
AppendMesh::addInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setNumberOfInputPorts(getNumInputPorts() + 1);
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
}