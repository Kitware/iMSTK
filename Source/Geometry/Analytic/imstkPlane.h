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
/// \class Plane
///
/// \brief Plane geometry
///
class Plane : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Plane() : AnalyticalGeometry(Type::Plane) {}

    ///
    /// \brief Default destructor
    ///
    ~Plane() = default;

    ///
    /// \brief Print the plane info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume (=0.0) for the plane
    ///
    double getVolume() const override;

    ///
    /// \brief Returns the normal of the plane
    ///
    Vec3d getNormal(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the normal to the plane
    ///
    void setNormal(const Vec3d n);
    void setNormal(double x, double y, double z);

    ///
    /// \brief Returns the width of the plane
    ///
    double getWidth(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the width of the plane
    ///
    void setWidth(const double w);

protected:
    friend class VTKPlaneRenderDelegate;

    void applyScaling(const double s) override;
    void updatePostTransformData() override;

    double m_width = 1.0;               ///> Width of the plane
    double m_widthPostTransform = 1.0;  ///> Width of the plane once transform applied
};
} // imstk
