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

// imstk
#include "imstkAnalyticalGeometry.h"

namespace imstk
{
///
/// \class Sphere
///
/// \brief Sphere geometry
///
class Sphere : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Sphere(const std::string name = std::string("")) : AnalyticalGeometry(Type::Sphere, name) {}

    ///
    /// \brief Print the sphere info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the sphere
    ///
    double getVolume() const override;

    ///
    /// \brief Returns the radius of the sphere
    ///
    double getRadius(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the radius of the sphere
    ///
    void setRadius(const double r);

    ///
    /// \brief Compute the bounding box for the geometry
    ///
    virtual void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

protected:
    friend class VTKSphereRenderDelegate;

    void applyScaling(const double s) override;
    void updatePostTransformData() const override;

    double m_radius = 1.0;                      ///> Radius of the sphere
    mutable double m_radiusPostTransform = 1.0; ///> Radius of the sphere once transform applied
};
} // imstk
