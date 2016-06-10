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

#include "imstkPenaltyRigidCH.h"

#include "imstkCollidingObject.h"
#include "imstkVirtualCouplingObject.h"
#include "imstkCollisionData.h"

#include <g3log/g3log.hpp>

namespace imstk {

void
PenaltyRigidCH::computeContactForces()
{
    auto movableObj = std::dynamic_pointer_cast<VirtualCouplingObject>(m_obj);
    if(movableObj == nullptr)
    {
        LOG(WARNING) << "PenaltyRigidCH::computeContactForces error: "
                     << m_obj->getName() << " is not a virtualcoupling object. "
                     << "(Rigid not yet supported, coming soon)";
        return;
    }

    // Get current force applied
    Vec3d force = movableObj->getForce();

    // If collision data, append forces
    if(!m_colData.PDColData.empty())
    {
        for(const auto& cd : m_colData.PDColData)
        {
            if (m_side == CollisionHandling::Side::A)
            {
                force -= cd.dirAtoB * ((cd.penetrationDepth+1)*(cd.penetrationDepth+1)-1)*10;
            }
            else if (m_side == CollisionHandling::Side::B)
            {
                force += cd.dirAtoB * ((cd.penetrationDepth+1)*(cd.penetrationDepth+1)-1)*10;
            }
        }
    }

    // Update object force
    movableObj->setForce(force);
}

}
