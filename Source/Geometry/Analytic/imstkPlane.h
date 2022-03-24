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
/// \class Plane
///
/// \brief Represents and infinite plane, width can be used for visual purposes
///
class Plane : public AnalyticalGeometry
{
public:
    Plane(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const Vec3d& normal = Vec3d(0.0, 1.0, 0.0)) :
        AnalyticalGeometry(), m_width(1.0)
    {
        setPosition(pos);
        setNormal(normal.normalized());
        updatePostTransformData();
    }

    ~Plane() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const override { return "Plane"; }

    ///
    /// \brief Get/Set the normal to the plane
    ///@{
    Vec3d getNormal(DataType type = DataType::PostTransform);
    void setNormal(const Vec3d n);
    void setNormal(const double x, const double y, const double z);
    ///@}

    ///
    /// \brief Get/Set the width of the plane, only used for visual purposes
    ///@{
    double getWidth();
    void setWidth(const double w);
    ///@}

    ///
    /// \brief Returns signed distance to surface at pos
    ///
    double getFunctionValue(const Vec3d& pos) const override { return m_normalPostTransform.dot(pos - m_positionPostTransform); }

    ///
    /// \brief Get the min, max of the AABB around the plane
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) override;

    ///
    /// \brief Update the Plane parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    Vec3d m_normal;
    mutable Vec3d m_normalPostTransform;
    double m_width; ///> Width of plane, only used for visual purposes
};
} // namespace imstk