/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneUtils.h"

namespace imstk
{
namespace SceneUtils
{
EntityPtr
makePbdEntity(const std::string& name, GeometryPtr visualGeometry, GeometryPtr collidingGeometry, GeometryPtr physicsGeometry, PbdSystemPtr system)
{
    auto entity = std::make_shared<Entity>(name);
    if (visualGeometry)
    {
        entity->addComponent<VisualModel>(name + "_VisualModel")->setGeometry(visualGeometry);
    }

    if (collidingGeometry)
    {
        entity->addComponent<Collider>(name + "_Collider")->setGeometry(collidingGeometry);
    }

    if (physicsGeometry)
    {
        auto method = entity->addComponent<PbdMethod>(name + "_PbdMethod");
        method->setPhysicsGeometry(physicsGeometry);
        method->setPbdSystem(system);
    }

    if (entity->getComponents().empty())
    {
        return nullptr;
    }

    return entity;
}

EntityPtr
makePbdEntity(const std::string& name, GeometryPtr geom, PbdSystemPtr system)
{
    return makePbdEntity(name, geom, geom, geom, system);
}
} // namespace SceneUtils
} // namespace imstk