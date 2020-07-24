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
    SPHHemorrhage(const Vec3d& center, const double radius, const Vec3d& normal);

    ///
    /// \brief Compute the cross-sectional area of the hemorrhage plane
    ///
    void computeHemorrhagePlaneArea();

    ///
    /// \brief Determine if fluid particle crossed the hemorrhage plane
    ///
    bool pointCrossedHemorrhagePlane(const Vec3d& oldPosition, const Vec3d& newPosition);

    const Vec3d getNormal() { return m_normal; }

    const double getHemorrhagePlaneArea() { return m_hemorrhagePlaneArea; }

    const double getHemorrhageRate() { return m_hemorrhageRate; }
    void setHemorrhageRate(const double hemorrhageRate) { m_hemorrhageRate = hemorrhageRate; }


private:
    Vec3d m_center;
    double m_radius;
    Vec3d m_normal;

    double m_hemorrhageRate = 0;

    double m_hemorrhagePlaneArea;
};
} // end namespace imstk
