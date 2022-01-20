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

#include "imstkAnalyticalGeometry.h"

namespace imstk
{
///
/// \class OrientedBox
///
/// \brief OrientedBox geometry, specified with extents (half lengths)
///
class OrientedBox : public AnalyticalGeometry
{
public:
    OrientedBox(const Vec3d& pos         = Vec3d::Zero(),
                const Vec3d extents      = Vec3d(0.5, 0.5, 0.5),
                const Quatd& orientation = Quatd::Identity(),
                const std::string& name  = std::string("defaultOrientedBox")) : AnalyticalGeometry(name)
    {
        setPosition(pos);
        setOrientation(orientation);
        setExtents(extents);
    }

    ~OrientedBox() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const override { return "OrientedBox"; }

    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() override { return m_extents[0] * m_extents[1] * m_extents[2] * 8.0; }

    ///
    /// \brief Returns the extents of the cube
    ///
    Vec3d getExtents(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the extents of the cube (half dimensions)
    ///
    void setExtents(const Vec3d extents);

    ///
    /// \brief Sets the width of the cube (half dimensions)
    ///
    void setExtents(const double x, const double y, const double z) { setExtents(Vec3d(x, y, z)); }

    ///
    /// \brief Returns signed distance to surface at pos
    ///
    double getFunctionValue(const Vec3d& pos) const override;

    ///
    /// \brief Get the min, max of the AABB around the cube
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) override;

    ///
    /// \brief Update the Cube parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    Vec3d m_extents = Vec3d(0.5, 0.5, 0.5);                      // Half dimensions of the oriented box
    mutable Vec3d m_extentsPostTransform = Vec3d(0.5, 0.5, 0.5); ///> Half dimensions of the oriented box once transform applied
};
} // namespace imstk
