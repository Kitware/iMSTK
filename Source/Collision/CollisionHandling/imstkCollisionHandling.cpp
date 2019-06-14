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

#include "imstkPenaltyCH.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkPickingCH.h"
#include "imstkDeformableObject.h"
#include "imstkBoneDrillingCH.h"
#include "imstkSPHCollisionHandling.h"

#include <g3log/g3log.hpp>

namespace imstk
{
std::shared_ptr<CollisionHandling>
CollisionHandling::make_collision_handling(const Type& type,
                                           const Side& side,
                                           const std::shared_ptr<CollisionData> colData,
                                           std::shared_ptr<CollidingObject> objA,
                                           std::shared_ptr<CollidingObject> objB)
{
    if (type != Type::None &&
        objA->getType() == SceneObject::Type::Visual)
    {
        LOG(WARNING) << "CollisionHandling::make_collision_handling error: "
                     << "penalty collision handling only implemented for colliding objects.";
        return nullptr;
    }

    switch (type)
    {
    case Type::None:
        return nullptr;

    case Type::Penalty:
        if (objA->getType() == SceneObject::Type::Visual)
        {
            LOG(WARNING) << "CollisionHandling::make_collision_handling error: "
                         << "penalty collision handling only implemented for colliding objects.";
            return nullptr;
        }

        return std::make_shared<PenaltyCH>(side, colData, objA);

    case Type::VirtualCoupling:

        return std::make_shared<VirtualCouplingCH>(side, colData, objA );

    case Type::NodalPicking:

        if (objA->getType() == SceneObject::Type::Visual)
        {
            LOG(WARNING) << "CollisionHandling::make_collision_handling error: "
                         << "penalty collision handling only implemented for colliding objects.";
            return nullptr;
        }
        if (auto defObj = std::dynamic_pointer_cast<DeformableObject>(objA))
        {
            return std::make_shared<PickingCH>(side, colData, defObj);
        }

    case Type::BoneDrilling:
        return std::make_shared<BoneDrillingCH>(side, colData, objA, objB);

    case Type::SPH:
        return std::make_shared<SPHCollisionHandling>(side, colData, objA);

    default:
        LOG(WARNING) << "CollisionHandling::make_collision_handling error: type not implemented.";
        return nullptr;
    }
}
}
