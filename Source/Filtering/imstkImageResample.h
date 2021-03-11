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
#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkMath.h"

namespace imstk
{
class ImageData;

///
/// \class ImageResample
///
/// \brief Trilinearly Resamples a 3d image to different dimensions
///
class ImageResample : public GeometryAlgorithm
{
public:
    ImageResample();

    virtual ~ImageResample() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    void setInputImage(std::shared_ptr<ImageData> inputData);

    imstkGetMacro(Dimensions, const Vec3i&);

    imstkSetMacro(Dimensions, const Vec3i&);

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions;
};
}