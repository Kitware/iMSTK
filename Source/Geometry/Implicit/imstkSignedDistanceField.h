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

#include "imstkImplicitGeometry.h"

namespace imstk
{
class ImageData;
class SurfaceMesh;
template<typename T> class DataArray;

///
/// \class SignedDistanceField
///
/// \brief Structured field of signed distances implemented with ImageData
///
class SignedDistanceField : public ImplicitGeometry
{
public:
    ///
    /// \brief Initialize with an image data SDF
    ///
    SignedDistanceField(std::shared_ptr<ImageData> imageData, std::string name = "");
    virtual ~SignedDistanceField() override = default;

public:
    ///
    /// \brief Returns signed distance to surface at pos, returns inf value if out of bounds of the SDF
    ///
    virtual double getFunctionValue(const Vec3d& pos) const override;

    ///
    /// \brief Returns gradient of signed distance field at pos
    ///
    virtual Vec3d getFunctionGrad(const Vec3d& pos) const override;

protected:
    std::shared_ptr<ImageData> m_imageDataSdf;
    std::shared_ptr<ImageData> m_imageDataGradient;
    Vec3d invSpacing;
    Vec6d bounds;
    std::shared_ptr<DataArray<float>> m_scalars;
};
}