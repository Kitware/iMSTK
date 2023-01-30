/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkObjectControllerGhost.h"
#include "imstkGeometry.h"
#include "imstkPbdObjectController.h"
#include "imstkRenderMaterial.h"
#include "imstkSceneObject.h"
#include "imstkVisualModel.h"

namespace imstk
{
ObjectControllerGhost::ObjectControllerGhost(const std::string& name) : SceneBehaviour(name),
    m_ghostVisualModel(std::make_shared<VisualModel>("GhostVisualModel"))
{
    m_ghostVisualModel->getRenderMaterial()->setColor(Color::Orange);
    m_ghostVisualModel->getRenderMaterial()->setLineWidth(5.0);
    m_ghostVisualModel->getRenderMaterial()->setOpacity(0.3);
    m_ghostVisualModel->getRenderMaterial()->setIsDynamicMesh(false);
}

void
ObjectControllerGhost::init()
{
    // Add a visual representation for the object
    // how to avoid adding it twice?
    std::shared_ptr<Entity> entity = m_entity.lock();
    CHECK(entity != nullptr) << "ObjectControllerGhost must have entity to initialize";
    if (!entity->containsComponent(m_ghostVisualModel))
    {
        m_ghostVisualModel->setName(entity->getName() + "_GhostVisualModel");
        entity->addComponent(m_ghostVisualModel);
    }

    CHECK(m_pbdController != nullptr)
        << "ObjectControllerGhost must have a controller";

    // Copy the geometry to the ghost visual model
    auto                      controlledObj = m_pbdController->getControlledObject();
    std::shared_ptr<Geometry> ghostGeom     = controlledObj->getVisualGeometry()->clone();
    CHECK(ghostGeom != nullptr) << "Failed to copy controller geometry";
    m_ghostVisualModel->setGeometry(ghostGeom);
}

void
ObjectControllerGhost::visualUpdate(const double&)
{
    Quatd orientation = Quatd::Identity();
    Vec3d position    = Vec3d::Zero();
    Vec3d force       = Vec3d::Zero();
    if (m_pbdController != nullptr)
    {
        orientation = m_pbdController->getOrientation();
        position    = m_pbdController->getPosition();
        force       = m_pbdController->getDeviceForce();
    }

    // Update the ghost debug geometry
    std::shared_ptr<Geometry> toolGhostMesh = m_ghostVisualModel->getGeometry();
    toolGhostMesh->setRotation(orientation);
    toolGhostMesh->setTranslation(position);
    toolGhostMesh->updatePostTransformData();
    toolGhostMesh->postModified();

    if (m_useForceFade)
    {
        // Could be desirable to set based off maximum device force (if you could query for it)
        m_ghostVisualModel->getRenderMaterial()->setOpacity(std::min(1.0, force.norm() / 15.0));
    }
}
} // namespace imstk