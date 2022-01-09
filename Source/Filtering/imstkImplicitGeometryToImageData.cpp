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

#include "imstkImplicitGeometryToImageData.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkGeometryUtilities.h"
#include "imstkImplicitGeometry.h"
#include "imstkDataArray.h"

namespace imstk
{
ImplicitGeometryToImageData::ImplicitGeometryToImageData()
{
    setNumInputPorts(1);
    setRequiredInputType<ImplicitGeometry>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<ImageData>());
}

std::shared_ptr<ImageData>
ImplicitGeometryToImageData::getOutputImage() const
{
    return std::dynamic_pointer_cast<ImageData>(getOutput(0));
}

void
ImplicitGeometryToImageData::setInputGeometry(std::shared_ptr<ImplicitGeometry> inputGeometry)
{
    setInput(inputGeometry, 0);
}

void
ImplicitGeometryToImageData::requestUpdate()
{
    std::shared_ptr<ImplicitGeometry> inputGeometry = std::dynamic_pointer_cast<ImplicitGeometry>(getInput(0));
    if (inputGeometry == nullptr)
    {
        LOG(WARNING) << "No inputGeometry to rasterize";
        return;
    }

    if (m_Dimensions[0] == 0 || m_Dimensions[1] == 0 || m_Dimensions[2] == 0)
    {
        LOG(WARNING) << "Dimensions must be non-zero";
        return;
    }

    const Vec3d size    = Vec3d(m_Bounds[1] - m_Bounds[0], m_Bounds[3] - m_Bounds[2], m_Bounds[5] - m_Bounds[4]);
    const Vec3d spacing = size.cwiseQuotient(m_Dimensions.cast<double>());
    const Vec3d origin  = Vec3d(m_Bounds[0], m_Bounds[2], m_Bounds[4]);
    const Vec3d shift   = origin + spacing * 0.5;

    std::shared_ptr<ImageData> outputImage = std::make_shared<ImageData>();
    outputImage->allocate(IMSTK_DOUBLE, 1, m_Dimensions, spacing, origin);
    DataArray<double>& scalars = *std::dynamic_pointer_cast<DataArray<double>>(outputImage->getScalars());
    double*            imgPtr  = scalars.getPointer();

    int i = 0;
    for (int z = 0; z < m_Dimensions[2]; z++)
    {
        for (int y = 0; y < m_Dimensions[1]; y++)
        {
            for (int x = 0; x < m_Dimensions[0]; x++, i++)
            {
                const Vec3d pos = Vec3d(x, y, z).cwiseProduct(spacing) + shift;
                imgPtr[i] = inputGeometry->getFunctionValue(pos);
            }
        }
    }

    setOutput(outputImage);
}
}