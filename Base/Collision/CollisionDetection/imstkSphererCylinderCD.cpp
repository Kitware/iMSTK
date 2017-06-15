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

#include "imstkSphereCylinderCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkSphere.h"

#include <g3log/g3log.hpp>

namespace imstk {
void
SphereCylinderCD::computeCollisionData()
{
    // Clear collisionData
    m_colData.clearAll();

    // Get geometry properties
    const Vec3d spherePos = m_sphere->getPosition();
    const double rSphere = m_sphere->getRadius();

    const Vec3d cylinderPos = m_cylinder->getPosition();
    const Vec3d cylinderAxis = m_cylinder->getOrientationAxis();
    const double rCylinder = m_cylinder->getRadius();

    // Compute shortest distance
    Vec3d distVec = (spherePos - cylinderPos) - cylinderAxis*(spherePos - cylinderPos).dot(cylinderAxis);
    Vec3d n = -distVec / distVec.norm();

    // Compute penetration depth
    double penetrationDepth = distVec.norm() - rSphere - rCylinder;
    if (penetrationDepth > 0.0)
    {
        return;
    }

    // Compute collision points
    Vec3d sphereColPt = spherePos + rSphere*n;
    Vec3d cylinderColPt = cylinderPos + cylinderAxis*(spherePos - cylinderPos).dot(cylinderAxis) + n * rCylinder;

    // Set collisionData
    m_colData.PDColData.push_back({ sphereColPt, cylinderColPt, n, penetrationDepth });
}
} //imstk
