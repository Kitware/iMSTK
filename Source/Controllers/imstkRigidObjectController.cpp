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

#include "imstkRigidObjectController.h"
#include "imstkCollidingObject.h"
#include "imstkDeviceClient.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"

namespace imstk
{
namespace expiremental
{
RigidObjectController::RigidObjectController(std::shared_ptr<RigidObject2> rigidObject,
                                             std::shared_ptr<DeviceClient> trackingDevice) :
    SceneObjectController(rigidObject, trackingDevice),
    m_rigidObject(rigidObject)
{
    /*m_currentPos = rigidObject->getRigidBody()->getPosition();
    m_currentRot = rigidObject->getRigidBody()->getOrientation();*/
}

void
RigidObjectController::updateControlledObjects()
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData())
        {
            LOG(WARNING) << "SceneObjectController::updateControlledObjects warning: could not update tracking info.";
            return;
        }
    }

    emit(Event(EventType::Modified));

    // During initialization tracking may not be enabled for a time, in which case, freeze the thing
    // or else extraneous forces may be applied towards uninitialized position (0, 0, 0) or gravity
    // pull it down
    if (!m_deviceClient->getTrackingEnabled())
    {
        (*m_rigidObject->getRigidBody()->m_pos) = m_rigidObject->getRigidBody()->m_initPos;
        (*m_rigidObject->getRigidBody()->m_orientation) = m_rigidObject->getRigidBody()->m_initOrientation;
        return;
    }

    // Apply virtual coupling
    const Vec3d currPos    = m_rigidObject->getRigidBody()->getPosition();
    const Vec3d desiredPos = getPosition();
    {
        const Vec3d diff = desiredPos - currPos;
        //const double length = diff.norm();
        //const Vec3d  dir    = diff.normalized();

        const Vec3d fS = m_linearKs.cwiseProduct(diff);
        const Vec3d fD = m_rigidObject->getRigidBody()->getVelocity() * -m_linearKd;

        // Apply spring force and general damper
        (*m_rigidObject->getRigidBody()->m_force) += (fS + fD);
    }

    const Quatd currOrientation    = m_rigidObject->getRigidBody()->getOrientation();
    const Quatd desiredOrientation = getRotation();
    {
        // q gives the delta rotation that gets us from curr->desired
        const Quatd dq = desiredOrientation * currOrientation.inverse();
        // Get the rotation axes
        Rotd angleAxes = Rotd(dq);
        angleAxes.axis().normalize();

        // Because the scale of the *magnitude* of the rotation done is controllable
        // it doesn't matter if its exact, ksTheta can just be adjusted
        (*m_rigidObject->getRigidBody()->m_torque) += (angleAxes.axis() * angleAxes.angle()).cwiseProduct(m_rotKs);
    }
}

void
RigidObjectController::applyForces()
{
    // Apply force back to device
    m_deviceClient->setForce(m_rigidObject->getRigidBody()->getForce());
    //m_deviceClient->setTorque();
}
}
}
