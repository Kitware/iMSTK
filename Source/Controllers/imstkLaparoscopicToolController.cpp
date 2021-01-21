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
    m_shaftVisualTransform    =  mat4dTranslate(m_shaft->getVisualGeometry()->getTranslation()) * mat4dRotation(m_shaft->getVisualGeometry()->getRotation());
    m_upperJawVisualTransform = mat4dTranslate(m_upperJaw->getVisualGeometry()->getTranslation()) * mat4dRotation(m_upperJaw->getVisualGeometry()->getRotation());
    m_lowerJawVisualTransform = mat4dTranslate(m_lowerJaw->getVisualGeometry()->getTranslation()) * mat4dRotation(m_lowerJaw->getVisualGeometry()->getRotation());

    m_shaftCollidingTransform    = mat4dTranslate(m_shaft->getCollidingGeometry()->getTranslation()) * mat4dRotation(m_shaft->getCollidingGeometry()->getRotation());
    m_upperJawCollidingTransform = mat4dTranslate(m_upperJaw->getCollidingGeometry()->getTranslation()) * mat4dRotation(m_upperJaw->getCollidingGeometry()->getRotation());
    m_lowerJawCollidingTransform = mat4dTranslate(m_lowerJaw->getCollidingGeometry()->getTranslation()) * mat4dRotation(m_lowerJaw->getCollidingGeometry()->getRotation());
}

void
LaparoscopicToolController::update(const double dt)
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData(dt))
        {
            LOG(WARNING) << "LaparoscopicToolController::updateControlledObjects warning: could not update tracking info.";
            return;
        }
    }

    const Vec3d controllerPosition    = getPosition();
    const Quatd controllerOrientation = getRotation();

    // Controller transform
    m_controllerWorldTransform = mat4dTranslate(controllerPosition) * mat4dRotation(controllerOrientation);

    // TRS decompose and set shaft geometries
    {
        Vec3d t, s;
        Mat3d r;

        mat4dTRS(m_controllerWorldTransform * m_shaftVisualTransform, t, r, s);
        m_shaft->getVisualGeometry()->setRotation(r);
        m_shaft->getVisualGeometry()->setTranslation(t);

        mat4dTRS(m_controllerWorldTransform * m_shaftCollidingTransform, t, r, s);
        m_shaft->getCollidingGeometry()->setRotation(r);
        m_shaft->getCollidingGeometry()->setTranslation(t);
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
        Vec3d t, s;
        Mat3d r;

        mat4dTRS(m_controllerWorldTransform * m_upperJawLocalTransform * m_upperJawVisualTransform, t, r, s);
        m_upperJaw->getVisualGeometry()->setRotation(r);
        m_upperJaw->getVisualGeometry()->setTranslation(t);

        mat4dTRS(m_controllerWorldTransform * m_upperJawLocalTransform * m_upperJawCollidingTransform, t, r, s);
        m_upperJaw->getCollidingGeometry()->setRotation(r);
        m_upperJaw->getCollidingGeometry()->setTranslation(t);
    }
    {
        Vec3d t, s;
        Mat3d r;

        mat4dTRS(m_controllerWorldTransform * m_lowerJawLocalTransform * m_upperJawVisualTransform, t, r, s);
        m_lowerJaw->getVisualGeometry()->setRotation(r);
        m_lowerJaw->getVisualGeometry()->setTranslation(t);

        mat4dTRS(m_controllerWorldTransform * m_lowerJawLocalTransform * m_lowerJawCollidingTransform, t, r, s);
        m_lowerJaw->getCollidingGeometry()->setRotation(r);
        m_lowerJaw->getCollidingGeometry()->setTranslation(t);
    }
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
