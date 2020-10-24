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

#include "imstkMath.h"

namespace imstk
{
///
/// \class SPHHemorrhage
/// \brief Class that holds methods and members for hemorrhage model
///
class SPHHemorrhage
{
public:
    // constructor
    SPHHemorrhage(const Vec3d& center, const double radius, const double area, const Vec3d& normal);

    ///
    /// \brief Determine if fluid particle crossed the hemorrhage plane
    ///
    bool pointCrossedHemorrhagePlane(const Vec3d& oldPosition, const Vec3d& newPosition);

    ///
    /// \brief Get the outward normal to the hemorrhage plane
    ///
    const Vec3d getNormal() { return m_normal; }

    ///
    /// \brief Get the area of the hemorrhage plane
    ///
    const double getHemorrhagePlaneArea() { return m_area; }

    ///
    /// \brief Get the rate of hemorrhage from SPH
    ///
    const double getHemorrhageRate() { return m_hemorrhageRate; }

    ///
    /// \brief Set the rate of hemorrhage from SPH
    ///
    void setHemorrhageRate(const double hemorrhageRate) { m_hemorrhageRate = hemorrhageRate; }

private:
    double m_area;                    ///> cross-sectional area of hemorrhage plane
    Vec3d  m_center;                  ///> hemorrhage plane center
    double m_hemorrhageRate = 0;      ///> rate of hemorrhage
    Vec3d  m_normal;                  ///> outward unit normal of hemorrhage plane
    double m_radius;                  ///> hemorrhage plane radius
};
} // end namespace imstk
