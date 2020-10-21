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
    std::shared_ptr<SceneObject>  shaft,
    std::shared_ptr<SceneObject>  upperJaw,
    std::shared_ptr<SceneObject>  lowerJaw,
    std::shared_ptr<DeviceClient> trackingDevice) :
    TrackingDeviceControl(trackingDevice),
    m_shaft(shaft),
    m_upperJaw(upperJaw),
    m_lowerJaw(lowerJaw),
    m_jawRotationAxis(Vec3d(0, 1., 0))
{
    trackingDevice->setButtonsEnabled(true);
}

void
LaparoscopicToolController::updateControlledObjects()
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData())
        {
            LOG(WARNING) << "LaparoscopicToolController::updateControlledObjects warning: could not update tracking info.";
            return;
        }
    }

    Vec3d p = getPosition();
    Quatd r = getRotation();

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

    // Update orientation of parts
    Quatd jawRotUpper;
    jawRotUpper = r * Rotd(m_jawAngle, m_jawRotationAxis);
    m_upperJaw->getMasterGeometry()->setRotation(jawRotUpper);

    Quatd jawRotLower;
    jawRotLower = r * Rotd(-m_jawAngle, m_jawRotationAxis);
    m_lowerJaw->getMasterGeometry()->setRotation(jawRotLower);

    m_shaft->getMasterGeometry()->setRotation(r);

    // Update positions of parts
    m_shaft->getMasterGeometry()->setTranslation(p);
    m_upperJaw->getMasterGeometry()->setTranslation(p);
    m_lowerJaw->getMasterGeometry()->setTranslation(p);
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
