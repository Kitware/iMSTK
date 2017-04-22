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

#include "imstkVirtualCouplingCH.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkAnalyticalGeometry.h"

#include <g3log/g3log.hpp>

namespace imstk
{

void
VirtualCouplingCH::computeContactForces()
{
    const auto collidingGeometry = std::static_pointer_cast<AnalyticalGeometry>(m_object->getCollidingGeometry());
    const auto visualGeometry =  std::static_pointer_cast<AnalyticalGeometry>(m_object->getVisualGeometry());

    // Check if any collisions
    const auto collidingObjPos = collidingGeometry->getPosition();
    if (m_colData.PDColData.empty())
    {
        // Set the visual object position same as the colliding object position
        visualGeometry->setPosition(collidingObjPos);
        return;
    }

    // Aggregate collision data
    Vec3d t = Vec3d::Zero();
    for (const auto& cd : m_colData.PDColData)
    {
        t += cd.dirAtoB * cd.penetrationDepth;
    }

    // Update the visual object position
    const auto visualObjPos = collidingObjPos + t;
    visualGeometry->setPosition(visualObjPos);

    // Spring force
    Vec3d  force = m_stiffness * (visualObjPos - collidingObjPos);

    // Damping force
    const double dt = 0.1; // Time step size to calculate the object velocity
    force += m_initialStep ? Vec3d(0.0, 0.0, 0.0) : m_damping * (collidingObjPos - m_prevPos) / dt;

    // Update object contact force
    m_object->appendForce(force);

    // Housekeeping
    m_initialStep = false;
    m_prevPos = collidingObjPos;
}

}// iMSTK
