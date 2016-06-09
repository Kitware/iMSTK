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

#include "imstkCollisionHandling.h"

#include "imstkPenaltyRigidCH.h"

#include <g3log/g3log.hpp>

namespace imstk {

std::shared_ptr<CollisionHandling>
CollisionHandling::make_collision_handling(const Type& type,
                                           std::shared_ptr<CollidingObject> objA,
                                           CollisionData &CDA,
                                           std::shared_ptr<CollidingObject> objB)
{
    switch (type)
    {
    case Type::Penalty:
    {
        if (objA->getType() != SceneObject::Type::VirtualCoupling &&
            objA->getType() == SceneObject::Type::Rigid)
        {
            LOG(WARNING) << "CollisionHandling::make_collision_handling error: "
                         << "penalty collision handling not yet implemented for non-rigid objects.";
            return nullptr;
        }
        return std::make_shared<PenaltyRigidCH>(objA, CDA);
    }break;

    default:
    {
        LOG(WARNING) << "CollisionHandling::make_collision_handling error: type not implemented.";
        return nullptr;
    }
    }
}

}
