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
    Cube(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double width = 1.0, const Vec3d& orientationAxis = Vec3d(0.0, 1.0, 0.0),
         const std::string& name = std::string("defaultCube")) : AnalyticalGeometry(name)
    {
        setPosition(pos);
        setOrientationAxis(orientationAxis);
        setWidth(width);
    }

    virtual ~Cube() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "Cube"; }

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
        // Unrotate the point and cube to axes align the cube
        const Mat3d r      = Quatd::FromTwoVectors(m_orientationAxisPostTransform, Vec3d(0.0, 1.0, 0.0)).toRotationMatrix();
        const Vec3d p      = r * pos;
        const Vec3d center = r * m_positionPostTransform;

        // Then test
        const Vec3d dmin = p - center - Vec3d(m_widthPostTransform, m_widthPostTransform, m_widthPostTransform) * 0.5;
        const Vec3d dmax = center - p - Vec3d(m_widthPostTransform, m_widthPostTransform, m_widthPostTransform) * 0.5;
        const Vec3d d    = dmin.cwiseMax(dmax);
        return std::max(std::max(d[0], d[1]), d[2]);
    }

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

    double m_width = 1.0;                      ///> Width of the cube
    mutable double m_widthPostTransform = 1.0; ///> Width of the cube once transform applied
};
}
