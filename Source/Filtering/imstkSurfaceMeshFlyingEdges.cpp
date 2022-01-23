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

#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkFlyingEdges3D.h>
#include <vtkImageData.h>

namespace imstk
{
SurfaceMeshFlyingEdges::SurfaceMeshFlyingEdges()
{
    setNumInputPorts(1);
    setRequiredInputType<ImageData>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

void
SurfaceMeshFlyingEdges::setInputImage(std::shared_ptr<ImageData> inputImage)
{
    setInput(inputImage, 0);
}

std::shared_ptr<SurfaceMesh>
SurfaceMeshFlyingEdges::getOutputMesh() const
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
SurfaceMeshFlyingEdges::requestUpdate()
{
    std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
    if (inputImage == nullptr)
    {
        LOG(WARNING) << "No inputImage to extract isosurface from";
        return;
    }

    vtkNew<vtkFlyingEdges3D> filter;
    filter->SetInputData(GeometryUtils::coupleVtkImageData(inputImage));
    filter->SetValue(0, m_IsoValue);
    filter->ComputeNormalsOff();
    filter->ComputeScalarsOff();
    filter->ComputeGradientsOff();
    filter->Update();

    std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0))->deepCopy(GeometryUtils::copyToSurfaceMesh(filter->GetOutput()));
}
} // namespace imstk