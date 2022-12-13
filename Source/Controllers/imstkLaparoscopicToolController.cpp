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
    m_shaft.sceneObject    = std::dynamic_pointer_cast<SceneObject>(shaft);
    m_upperJaw.sceneObject = std::dynamic_pointer_cast<SceneObject>(upperJaw);
    m_lowerJaw.sceneObject = std::dynamic_pointer_cast<SceneObject>(lowerJaw);
    m_pickGeom = pickGeom;
    m_jawRotationAxis = Vec3d(1, 0, 0);

    m_shaft.visualGeometry    = m_shaft.sceneObject->getVisualGeometry();
    m_upperJaw.visualGeometry = m_upperJaw.sceneObject->getVisualGeometry();
    m_lowerJaw.visualGeometry = m_lowerJaw.sceneObject->getVisualGeometry();

    m_shaft.collider    = m_shaft.sceneObject->getComponent<Collider>();
    m_upperJaw.collider = m_upperJaw.sceneObject->getComponent<Collider>();
    m_lowerJaw.collider = m_lowerJaw.sceneObject->getComponent<Collider>();

    // Record the transforms as 4x4 matrices (this should capture initial displacement/rotation of the jaws/shaft from controller)
    m_shaft.visualTransform    = m_shaft.visualGeometry->getTransform();
    m_upperJaw.visualTransform = m_upperJaw.visualGeometry->getTransform();
    m_lowerJaw.visualTransform = m_lowerJaw.visualGeometry->getTransform();

    m_shaft.collidingTransform    = m_shaft.collider->getGeometry()->getTransform();
    m_upperJaw.collidingTransform = m_upperJaw.collider->getGeometry()->getTransform();
    m_lowerJaw.collidingTransform = m_lowerJaw.collider->getGeometry()->getTransform();

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
        m_shaft.visualGeometry->setTransform(m_controllerWorldTransform * m_shaft.visualTransform);
        m_shaft.collider->getGeometry()->setTransform(m_controllerWorldTransform * m_shaft.collidingTransform);
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
    m_upperJaw.localTransform = mat4dRotation(Rotd(m_jawAngle, m_jawRotationAxis));
    m_lowerJaw.localTransform = mat4dRotation(Rotd(-m_jawAngle, m_jawRotationAxis));
    {
        const Mat4d upperWorldTransform = m_controllerWorldTransform * m_upperJaw.localTransform;
        m_upperJaw.visualGeometry->setTransform(upperWorldTransform * m_upperJaw.visualTransform);
        m_upperJaw.collider->getGeometry()->setTransform(upperWorldTransform * m_upperJaw.collidingTransform);
    }
    {
        const Mat4d lowerWorldTransform = m_controllerWorldTransform * m_lowerJaw.localTransform;
        m_lowerJaw.visualGeometry->setTransform(lowerWorldTransform * m_upperJaw.visualTransform);
        m_lowerJaw.collider->getGeometry()->setTransform(lowerWorldTransform * m_lowerJaw.collidingTransform);
    }
    m_shaft.visualGeometry->postModified();
    m_lowerJaw.visualGeometry->postModified();
    m_upperJaw.visualGeometry->postModified();

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