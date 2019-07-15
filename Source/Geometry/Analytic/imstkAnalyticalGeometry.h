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

#include "imstkGeometry.h"
#include "imstkMath.h"

namespace imstk
{
///
/// \class AnalyticalGeometry
///
/// \brief Base class for any analytical geometrical representation
///
class AnalyticalGeometry : public Geometry
{
public:

    ///
    /// \brief Destructor
    ///
    virtual ~AnalyticalGeometry() = default;

    ///
    /// \brief Print
    ///
    virtual void print() const override;

    // Accessors

    ///
    /// \brief Get/Set position
    ///
    Vec3d getPosition(DataType type = DataType::PostTransform);
    void setPosition(const Vec3d p);
    void setPosition(const double x, const double y, const double z);

    ///
    /// \brief Get/Set orientation axis
    ///
    Vec3d getOrientationAxis(DataType type = DataType::PostTransform);
    void setOrientationAxis(const Vec3d axis);

protected:

    AnalyticalGeometry(Type type) : Geometry(type) {}

    void applyTranslation(const Vec3d t) override;
    void applyRotation(const Mat3d r) override;
    virtual void updatePostTransformData() override;

    Vec3d m_position = WORLD_ORIGIN;                  ///> position
    Vec3d m_positionPostTransform = WORLD_ORIGIN;     ///> position once transform applied

    Vec3d m_orientationAxis = UP_VECTOR;              ///> orientation
    Vec3d m_orientationAxisPostTransform = UP_VECTOR; ///> orientation once transform applied
};
} //imstk
