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
/// \brief Cylinder geometry
///
class Cylinder : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    explicit Cylinder(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const double radius = 1.0, const double length = 1.0,
                      const Vec3d& orientationAxis = Vec3d(0.0, 1.0, 0.0), const std::string& name = std::string("defaultCylinder")) :
        AnalyticalGeometry(Type::Cylinder, name)
    {
        setPosition(pos);
        setOrientationAxis(orientationAxis);
        setRadius(radius);
        setLength(length);
        updatePostTransformData();
    }

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

protected:
    friend class VTKCylinderRenderDelegate;

    // Hide these unimplemented functions
    using AnalyticalGeometry::getFunctionValue;
    //using AnalyticalGeometry::getFunctionGrad;

    void applyScaling(const double s) override;
    void updatePostTransformData() const override;

    double m_radius = 1.0;                      ///> Radius of the cylinder
    double m_length = 1.0;                      ///> Length of the cylinder
    mutable double m_radiusPostTransform = 1.0; ///> Radius of the cylinder oncee transform applied
    mutable double m_lengthPostTransform = 1.0; ///> Length of the cylinder onc transform applied
};
} // imstk
