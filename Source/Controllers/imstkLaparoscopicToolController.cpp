/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLaparoscopicToolController.h"
#include "imstkCollider.h"
#include "imstkEntity.h"
#include "imstkDeviceClient.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkSceneObject.h"

namespace imstk
{
void
LaparoscopicToolController::setParts(
    std::shared_ptr<Entity>   shaft,
    std::shared_ptr<Entity>   upperJaw,
    std::shared_ptr<Entity>   lowerJaw,
    std::shared_ptr<Geometry> pickGeom)
{
    m_shaft    = std::dynamic_pointer_cast<SceneObject>(shaft);
    m_upperJaw = std::dynamic_pointer_cast<SceneObject>(upperJaw);
    m_lowerJaw = std::dynamic_pointer_cast<SceneObject>(lowerJaw);
    m_pickGeom = pickGeom;
    m_jawRotationAxis = Vec3d(1, 0, 0);

    // Record the transforms as 4x4 matrices (this should capture initial displacement/rotation of the jaws/shaft from controller)
    m_shaftVisualTransform    = m_shaft->getVisualGeometry()->getTransform();
    m_upperJawVisualTransform = m_upperJaw->getVisualGeometry()->getTransform();
    m_lowerJawVisualTransform = m_lowerJaw->getVisualGeometry()->getTransform();

    m_shaftCollidingTransform    = m_shaft->getComponent<Collider>()->getGeometry()->getTransform();
    m_upperJawCollidingTransform = m_upperJaw->getComponent<Collider>()->getGeometry()->getTransform();
    m_lowerJawCollidingTransform = m_lowerJaw->getComponent<Collider>()->getGeometry()->getTransform();

    m_pickGeomTransform = m_pickGeom->getTransform();
}

void
LaparoscopicToolController::setDevice(std::shared_ptr<DeviceClient> device)
{
    TrackingDeviceControl::setDevice(device);
    device->setButtonsEnabled(true);
}

void
LaparoscopicToolController::update(const double& dt)
{
    if (!updateTrackingData(dt))
    {
        LOG(WARNING) << "warning: could not update tracking info.";
        return;
    }

    const Vec3d& controllerPosition    = getPosition();
    const Quatd& controllerOrientation = getOrientation();

    // Controller transform
    m_controllerWorldTransform = mat4dTranslate(controllerPosition) * mat4dRotation(controllerOrientation);

    // Set shaft geometries
    {
        m_shaft->getVisualGeometry()->setTransform(m_controllerWorldTransform * m_shaftVisualTransform);
        m_shaft->getComponent<Collider>()->getGeometry()->setTransform(m_controllerWorldTransform * m_shaftCollidingTransform);
        m_pickGeom->setTransform(m_controllerWorldTransform * m_pickGeomTransform);
    }

    // Update jaw angles
    if (m_deviceClient->getButton(0))
    {
        m_jawAngle += m_change * dt;
    }
    if (m_deviceClient->getButton(1))
    {
        m_jawAngle -= m_change * dt;
    }

    // Clamp
    m_jawAngle = std::max(std::min(m_jawAngle, m_maxJawAngle), 0.0);

    // Update transforms
    m_upperJawLocalTransform = mat4dRotation(Rotd(m_jawAngle, m_jawRotationAxis));
    m_lowerJawLocalTransform = mat4dRotation(Rotd(-m_jawAngle, m_jawRotationAxis));
    {
        const Mat4d upperWorldTransform = m_controllerWorldTransform * m_upperJawLocalTransform;
        m_upperJaw->getVisualGeometry()->setTransform(upperWorldTransform * m_upperJawVisualTransform);
        m_upperJaw->getComponent<Collider>()->getGeometry()->setTransform(upperWorldTransform * m_upperJawCollidingTransform);
    }
    {
        const Mat4d lowerWorldTransform = m_controllerWorldTransform * m_lowerJawLocalTransform;
        m_lowerJaw->getVisualGeometry()->setTransform(lowerWorldTransform * m_upperJawVisualTransform);
        m_lowerJaw->getComponent<Collider>()->getGeometry()->setTransform(lowerWorldTransform * m_lowerJawCollidingTransform);
    }
    m_shaft->getVisualGeometry()->postModified();
    m_lowerJaw->getVisualGeometry()->postModified();
    m_upperJaw->getVisualGeometry()->postModified();

    // Check for transition closed/open
    if (m_jawState == JawState::Opened && m_jawAngle <= 0.0)
    {
        m_jawState = JawState::Closed;
        this->postEvent(Event(JawClosed()));
    }
    // When the jaw angle surpasses this degree it is considered open
    const double openingDegree = 5.0;
    if (m_jawState == JawState::Closed && m_jawAngle >= openingDegree * PI / 180.0)
    {
        m_jawState = JawState::Opened;
        this->postEvent(Event(JawOpened()));
    }
}
} // namespace imstk