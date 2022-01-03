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
/// \class Cylinder
///
/// \brief Cylinder geometry, default configuration is at origin with length running up the y axes
///
class Cylinder : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Cylinder(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double radius = 1.0, const double length = 1.0,
             const Quatd& orientation = Quatd::Identity(), const std::string& name = std::string("defaultCylinder")) :
        AnalyticalGeometry(name)
    {
        setPosition(pos);
        setOrientation(orientation);
        setRadius(radius);
        setLength(length);
        updatePostTransformData();
    }

    virtual ~Cylinder() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "Cylinder"; }

    ///
    /// \brief Print the cylinder info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cylinder
    ///
    double getVolume() override { return PI * m_radius * m_radius * m_length; }

    ///
    /// \brief Returns the radius of the cylinder
    ///
    double getRadius(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the radius of the cylinder
    ///
    void setRadius(const double r);

    ///
    /// \brief Returns the length of the cylinder
    ///
    double getLength(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the length of the cylinder
    ///
    void setLength(const double r);

    ///
    /// \brief Get the min, max of the AABB around the cylinder
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent);

    ///
    /// \brief Update the Cylinder parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    // Hide these unimplemented functions
    using AnalyticalGeometry::getFunctionValue;

    void applyTransform(const Mat4d& m) override;

    double m_radius = 1.0;                      ///> Radius of the cylinder
    double m_length = 1.0;                      ///> Length of the cylinder
    mutable double m_radiusPostTransform = 1.0; ///> Radius of the cylinder oncee transform applied
    mutable double m_lengthPostTransform = 1.0; ///> Length of the cylinder onc transform applied
};
} // imstk
