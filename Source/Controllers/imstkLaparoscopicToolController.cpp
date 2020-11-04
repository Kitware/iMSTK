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
    m_jawRotationAxis(Vec3d(1, 0, 0))
{
    trackingDevice->setButtonsEnabled(true);

    // Record the transforms as 4x4 matrices (this should capture initial displacement/rotation of the jaws from the shaft)
    m_shaftWorldTransform = mat4dRotation(m_shaft->getMasterGeometry()->getRotation()) * mat4dTranslate(m_shaft->getMasterGeometry()->getTranslation());

    m_upperJawLocalTransform = mat4dRotation(m_upperJaw->getMasterGeometry()->getRotation()) * mat4dTranslate(m_upperJaw->getMasterGeometry()->getTranslation());
    m_lowerJawLocalTransform = mat4dRotation(m_lowerJaw->getMasterGeometry()->getRotation()) * mat4dTranslate(m_lowerJaw->getMasterGeometry()->getTranslation());

    m_upperJawWorldTransform = m_shaftWorldTransform * m_upperJawLocalTransform;
    m_lowerJawWorldTransform = m_shaftWorldTransform * m_lowerJawLocalTransform;
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

    const Vec3d controllerPosition = getPosition();
    const Quatd controllerOrientation = getRotation();

    // Controller transform
    m_shaftWorldTransform = mat4dTranslate(controllerPosition) * mat4dRotation(controllerOrientation);
    {
        // TRS decompose and set shaft
        Vec3d t, s;
        Mat3d r;
        mat4dTRS(m_shaftWorldTransform, t, r, s);
        m_shaft->getMasterGeometry()->setRotation(r);
        m_shaft->getMasterGeometry()->setTranslation(t);
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
    m_upperJawWorldTransform = m_shaftWorldTransform * m_upperJawLocalTransform;
    m_lowerJawWorldTransform = m_shaftWorldTransform * m_lowerJawLocalTransform;

    {
        Vec3d t, s;
        Mat3d r;
        mat4dTRS(m_upperJawWorldTransform, t, r, s);
        m_upperJaw->getMasterGeometry()->setRotation(r);
        m_upperJaw->getMasterGeometry()->setTranslation(t);
    }
    {
        Vec3d t, s;
        Mat3d r;
        mat4dTRS(m_lowerJawWorldTransform, t, r, s);
        m_lowerJaw->getMasterGeometry()->setRotation(r);
        m_lowerJaw->getMasterGeometry()->setTranslation(t);
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
