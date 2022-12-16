/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollider.h"
#include "imstkEntity.h"
#include "imstkGeometry.h"
#include "imstkGeometryMap.h"

namespace imstk
{
std::shared_ptr<Geometry>
Collider::getCollidingGeometryFromEntity(Entity* entity)
{
    if (entity != nullptr)
    {
        if (auto collider = entity->getComponent<Collider>())
        {
            return collider->getGeometry();
        }
    }
    return nullptr;
}
} // namespace imstk
