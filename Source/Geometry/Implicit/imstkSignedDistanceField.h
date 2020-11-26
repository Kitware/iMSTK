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
template<typename T> class DataArray;

///
/// \class SignedDistanceField
///
/// \brief Structured field of signed distances implemented with ImageData
/// The SDF differs in that when you scale an image via its spacing the
/// distance samples are then wrong. Here you can isotropically scale as you
/// wish
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
    /// \brief Returns signed distance to surface at pos, returns clamped/nearest if out of bounds
    ///
    double getFunctionValue(const Vec3d& pos) const override;

    ///
    /// \brief Returns signed distance to surface at coordinate
    ///
    double getFunctionValueCoord(const Vec3i& coord) const;

    ///
    /// \brief Returns the bounds of the field
    ///
    const Vec6d& getBounds() const { return m_bounds; }

    ///
    /// \brief Set the isotropic scale that is used/multplied with samples
    ///
    void setScale(const double scale) { m_scale = scale; }

    ///
    /// \brief Get the isotropic scale
    ///
    double getScale() const { return m_scale; }

    ///
    /// \brief Get the SDF as a float image
    ///
    std::shared_ptr<ImageData> getImage() const { return m_imageDataSdf; }

protected:
    std::shared_ptr<ImageData> m_imageDataSdf;

    Vec3d  m_invSpacing;
    Vec6d  m_bounds;
    Vec3d  m_shift;
    double m_scale;

    std::shared_ptr<DataArray<double>> m_scalars;
};
}