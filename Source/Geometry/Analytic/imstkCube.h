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
class Cube;

///
/// \class Cube
///
/// \brief Cube geometry
///
class Cube : public AnalyticalGeometry
{
public:
    explicit Cube(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double width = 1.0, const Vec3d& orientationAxis = Vec3d(0.0, 1.0, 0.0),
                  const std::string& name = std::string("defaultCube")) : AnalyticalGeometry(Type::Cube, name)
    {
        setPosition(pos);
        setOrientationAxis(orientationAxis);
        setWidth(width);
    }

    ~Cube() override = default;

public:
    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() override { return m_width * m_width * m_width; }

    ///
    /// \brief Returns the width of the cube
    ///
    double getWidth(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the width of the cube
    ///
    void setWidth(const double w);

    ///
    /// \brief Returns signed distance to surface at pos
    /// \todo Doesn't support orientation yet
    ///
    double getFunctionValue(const Vec3d& pos) const override
    {
        const Vec3d dmin = pos - m_position - Vec3d(m_width, m_width, m_width) * 0.5;
        const Vec3d dmax = m_position - pos - Vec3d(m_width, m_width, m_width) * 0.5;
        const Vec3d d    = dmin.cwiseMax(dmax);
        return std::max(std::max(d[0], d[1]), d[2]);
    }

    ///
    /// \brief Get the min, max of the AABB around the cube
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) override;

protected:
    void applyTransform(const Mat4d& m) override;
    void updatePostTransformData() const override;

    double m_width = 1.0;                      ///> Width of the cube
    mutable double m_widthPostTransform = 1.0; ///> Width of the cube once transform applied
};
}
