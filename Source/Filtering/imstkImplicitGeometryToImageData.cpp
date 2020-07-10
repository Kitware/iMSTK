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
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
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

    const Vec3d size = Vec3d(Bounds[1] - Bounds[0], Bounds[3] - Bounds[2], Bounds[5] - Bounds[4]);
    const Vec3d spacing = size.cwiseQuotient(Dimensions.cast<double>());
    const Vec3d origin = Vec3d(Bounds[0], Bounds[2], Bounds[4]) + spacing * 0.5;

    std::shared_ptr<ImageData> outputImage = std::make_shared<ImageData>();
    outputImage->allocate(IMSTK_FLOAT, 1, Dimensions, spacing, origin);
    DataArray<float>& scalars = *std::dynamic_pointer_cast<DataArray<float>>(outputImage->getScalars());
    float* imgPtr = scalars.getPointer();

    int i = 0;
    for (int z = 0; z < Dimensions[2]; z++)
    {
        for (int y = 0; y < Dimensions[1]; y++)
        {
            for (int x = 0; x < Dimensions[0]; x++, i++)
            {
                const Vec3d pos = Vec3d(x, y, z).cwiseProduct(spacing) + origin;
                imgPtr[i] = inputGeometry->getFunctionValue(pos);
            }
        }
    }

    setOutput(outputImage);
}
}