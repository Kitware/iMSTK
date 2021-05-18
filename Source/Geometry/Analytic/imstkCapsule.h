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
/// \class Capsule
///
/// \brief Capsule geometry
///
class Capsule : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Capsule(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double radius = 0.5, const double length = 1.0, const Vec3d orientationAxis = Vec3d(0.0, 1.0, 0.0),
            const std::string& name = std::string("defaultCapsule")) :
        AnalyticalGeometry(name)
    {
        setPosition(pos);
        setOrientationAxis(orientationAxis);
        setRadius(radius);
        setLength(length);
    }

    ///
    /// \brief Deconstructor
    ///
    virtual ~Capsule() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "Capsule"; }

public:
    ///
    /// \brief Print the capsule info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the capsule
    ///
    double getVolume() override { return PI * m_radius * m_radius * (m_length + 4.0 / 3.0 * m_radius); }

    ///
    /// \brief Returns the radius of the capsule
    ///
    double getRadius(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the radius of the capsule
    ///
    void setRadius(const double r);

    ///
    /// \brief Returns the length of the capsule
    ///
    double getLength(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the length of the capsule
    ///
    void setLength(const double l);

    ///
    /// \brief Returns the signed distance to the capsule
    ///
    double getFunctionValue(const Vec3d& x) const override;

    ///
    /// \brief Get the min, max of the AABB around the capsule
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent);

    ///
    /// \brief Update the Capsule parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    double m_radius = 1.0;                      ///> Radius of the hemispheres at the end of the capsule
    mutable double m_radiusPostTransform = 1.0; ///> Radius after transform
    double m_length = 1.0;                      ///> Length between the centers of two hemispheres
    mutable double m_lengthPostTransform = 1.0; ///> Length after transform
};
} // imstk
