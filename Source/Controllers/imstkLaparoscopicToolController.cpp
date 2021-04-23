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

#include "imstkLaparoscopicToolController.h"
#include "imstkCollidingObject.h"
#include "imstkDeviceClient.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"

namespace imstk
{
LaparoscopicToolController::LaparoscopicToolController(
    std::shared_ptr<CollidingObject> shaft,
    std::shared_ptr<CollidingObject> upperJaw,
    std::shared_ptr<CollidingObject> lowerJaw,
    std::shared_ptr<DeviceClient>    trackingDevice) :
    TrackingDeviceControl(trackingDevice),
    m_shaft(shaft),
    m_upperJaw(upperJaw),
    m_lowerJaw(lowerJaw),
    m_jawRotationAxis(Vec3d(1, 0, 0))
{
    trackingDevice->setButtonsEnabled(true);

    // Record the transforms as 4x4 matrices (this should capture initial displacement/rotation of the jaws/shaft from controller)
    m_shaftVisualTransform    = m_shaft->getVisualGeometry()->getTransform();
    m_upperJawVisualTransform = m_upperJaw->getVisualGeometry()->getTransform();
    m_lowerJawVisualTransform = m_lowerJaw->getVisualGeometry()->getTransform();

    m_shaftCollidingTransform    = m_shaft->getCollidingGeometry()->getTransform();
    m_upperJawCollidingTransform = m_upperJaw->getCollidingGeometry()->getTransform();
    m_lowerJawCollidingTransform = m_lowerJaw->getCollidingGeometry()->getTransform();
}

void
LaparoscopicToolController::update(const double dt)
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData(dt))
        {
            LOG(WARNING) << "warning: could not update tracking info.";
            return;
        }
    }

    const Vec3d controllerPosition    = getPosition();
    const Quatd controllerOrientation = getRotation();

    // Controller transform
    m_controllerWorldTransform = mat4dTranslate(controllerPosition) * mat4dRotation(controllerOrientation);

    // Set shaft geometries
    {
        m_shaft->getVisualGeometry()->setTransform(m_controllerWorldTransform * m_shaftVisualTransform);
        m_shaft->getCollidingGeometry()->setTransform(m_controllerWorldTransform * m_shaftCollidingTransform);
    }

    // Update jaw angles
    if (m_deviceClient->getButton(0))
    {
        m_jawAngle += m_change;
        m_jawAngle  = (m_jawAngle > m_maxJawAngle) ? m_maxJawAngle : m_jawAngle;
    }
    if (m_deviceClient->getButton(1))
    {
        m_jawAngle -= m_change;
        m_jawAngle  = (m_jawAngle < 0.0) ? 0.0 : m_jawAngle;
    }

    m_upperJawLocalTransform = mat4dRotation(Rotd(m_jawAngle, m_jawRotationAxis));
    m_lowerJawLocalTransform = mat4dRotation(Rotd(-m_jawAngle, m_jawRotationAxis));

    // TRS decompose and set upper/lower jaw geometries
    {
        const Mat4d upperWorldTransform = m_controllerWorldTransform * m_upperJawLocalTransform;
        m_upperJaw->getVisualGeometry()->setTransform(upperWorldTransform * m_upperJawVisualTransform);
        m_upperJaw->getCollidingGeometry()->setTransform(upperWorldTransform * m_upperJawCollidingTransform);
    }
    {
        const Mat4d lowerWorldTransform = m_controllerWorldTransform * m_lowerJawLocalTransform;
        m_lowerJaw->getVisualGeometry()->setTransform(lowerWorldTransform * m_upperJawVisualTransform);
        m_lowerJaw->getCollidingGeometry()->setTransform(lowerWorldTransform * m_lowerJawCollidingTransform);
    }

    m_shaft->getVisualGeometry()->postModified();
    m_lowerJaw->getVisualGeometry()->postModified();
    m_upperJaw->getVisualGeometry()->postModified();
}

void
LaparoscopicToolController::applyForces()
{
    Vec3d force(0, 0, 0);

    if (auto collidingObject = dynamic_cast<CollidingObject*>(m_shaft.get()))
    {
        force += collidingObject->getForce();
    }

    if (auto collidingObject = dynamic_cast<CollidingObject*>(m_upperJaw.get()))
    {
        force += collidingObject->getForce();
    }

    if (auto collidingObject = dynamic_cast<CollidingObject*>(m_lowerJaw.get()))
    {
        force += collidingObject->getForce();
    }

    m_deviceClient->setForce(force);
}
} // imstk
