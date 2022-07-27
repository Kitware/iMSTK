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
///
/// \class AnalyticalGeometry
///
/// \brief Base class for any analytical geometrical representation
///
class AnalyticalGeometry : public ImplicitGeometry
{
public:
    ~AnalyticalGeometry() override = default;

    ///
    /// \brief Print
    ///
    void print() const override;

    // Accessors
    ///
    /// \brief Get the local or global position (post transformed)
    ///
    Vec3d getPosition(DataType type = DataType::PostTransform);

    ///
    /// \brief Set the local position
    ///@{
    void setPosition(const Vec3d p);
    void setPosition(const double x, const double y, const double z);
    ///@}

    ///
    /// \brief Get the local or global orientation (post transformed)
    ///
    Quatd getOrientation(DataType type = DataType::PostTransform);

    ///
    /// \brief Set the local orientation
    ///
    void setOrientation(const Quatd r);

    ///
    /// \brief Returns the implicit function value, this could signed, unsigned distance,
    /// or some other scalar.
    /// \param Position in global space
    /// Note: Called frequently and often in parallel
    ///
    double getFunctionValue(const Vec3d& imstkNotUsed(pos)) const override { return 0.0; }

    ///
    /// \brief Apply the global transform to the local parameters producing post
    /// transformed parameters.
    ///
    void updatePostTransformData() const override;

protected:
    AnalyticalGeometry();

    ///
    /// \brief Apply a user transform directly to (pre-transformed) parameters producing
    /// new parameters.
    ///
    void applyTransform(const Mat4d& m) override;

    Vec3d m_position;                         ///< position
    mutable Vec3d m_positionPostTransform;    ///< position once transform applied
    Quatd m_orientation;                      ///< orientation
    mutable Quatd m_orientationPostTransform; ///< orientation once transform is applied
};
} // namespace imstk