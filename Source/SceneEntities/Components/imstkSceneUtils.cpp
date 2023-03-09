/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRenderMaterial.h"
#include "imstkSceneUtils.h"

namespace imstk
{
namespace SceneUtils
{
std::shared_ptr<Entity>
makePbdEntity(const std::string& name, std::shared_ptr<Geometry> visualGeometry, std::shared_ptr<Geometry> collidingGeometry, std::shared_ptr<Geometry> physicsGeometry,
              std::shared_ptr<PbdSystem> system)
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
        method->setGeometry(physicsGeometry);
        method->setPbdSystem(system);
    }

    if (entity->getComponents().empty())
    {
        return nullptr;
    }

    return entity;
}

std::shared_ptr<Entity>
makePbdEntity(const std::string& name, std::shared_ptr<Geometry> geom, std::shared_ptr<PbdSystem> system)
{
    return makePbdEntity(name, geom, geom, geom, system);
}

std::shared_ptr<RenderMaterial>
makeTissueMaterial(double opacity)
{
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(opacity);
    return material;
}
} // namespace SceneUtils
} // namespace imstk