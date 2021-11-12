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
#include "imstkDeviceClient.h"
#include "imstkLogger.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"

namespace imstk
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
RigidObjectController::setControlledSceneObject(std::shared_ptr<SceneObject> obj)
{
    SceneObjectController::setControlledSceneObject(obj);
    m_rigidObject = std::dynamic_pointer_cast<RigidObject2>(obj);
}

void
RigidObjectController::update(const double dt)
{
    if (!isTrackerUpToDate())
    {
        if (!updateTrackingData(dt))
        {
            LOG(WARNING) << "warning: could not update tracking info.";
            return;
        }
    }

    if (m_rigidObject == nullptr)
    {
        return;
    }

    // Implementation based of otaduy lin's paper eq14
    // "A Modular Haptic Rendering Algorithm for Stable and Transparent 6 - DOF Manipulation"
    if (m_deviceClient->getTrackingEnabled() && m_useSpring)
    {
        const Vec3d& currPos = m_rigidObject->getRigidBody()->getPosition();
        const Quatd& currOrientation     = m_rigidObject->getRigidBody()->getOrientation();
        const Vec3d& currVelocity        = m_rigidObject->getRigidBody()->getVelocity();
        const Vec3d& currAngularVelocity = m_rigidObject->getRigidBody()->getAngularVelocity();
        Vec3d&       currForce  = *m_rigidObject->getRigidBody()->m_force;
        Vec3d&       currTorque = *m_rigidObject->getRigidBody()->m_torque;

        const Vec3d& devicePos = getPosition();
        const Quatd& deviceOrientation = getOrientation();
        //const Vec3d& deviceVelocity        = getVelocity();
        //const Vec3d& deviceAngularVelocity = getAngularVelocity();
        const Vec3d& deviceOffset = Vec3d(0.0, 0.0, 0.0);

        // Uses non-relative force
        {
            // Compute linear force
            fS = m_linearKs.cwiseProduct(devicePos - currPos - deviceOffset) + m_linearKd * (-currVelocity - currAngularVelocity.cross(deviceOffset));

            //printf("Device velocity %f, %f, %f\n", deviceVelocity[0], deviceVelocity[1], deviceVelocity[2]);
            const Quatd dq = deviceOrientation * currOrientation.inverse();
            const Rotd  angleAxes = Rotd(dq);
            tS = deviceOffset.cross(fS) + m_angularKs.cwiseProduct(angleAxes.axis() * angleAxes.angle()) + m_angularKd * -currAngularVelocity;

            currForce  += fS;
            currTorque += tS;
        }
        // Uses relative force
        //{
        //    // Compute force (?? Why does the spring use displacement, while damper uses velocity, these aren't relative, ie: fD could be larger than )
        //    const Vec3d dx = devicePos - currPos - deviceOffset;
        //    fS = m_linearKs.cwiseProduct(dx) + m_linearKd * (deviceVelocity - currVelocity - currAngularVelocity.cross(deviceOffset));

        //    // Compute torque
        //    const Quatd dq = deviceOrientation * currOrientation.inverse();
        //    const Rotd angleAxes = Rotd(dq);
        //    tS = deviceOffset.cross(fS) + m_angularKs.cwiseProduct(angleAxes.axis() * angleAxes.angle()); + m_angularKd * (deviceAngularVelocity - currAngularVelocity);

        //    // Apply to body
        //    currForce += fS;
        //    //std::cout << "fS: " << fS[0] << ", " << fS[1] << ", " << fS[2] << std::endl;
        //    currTorque += tS;
        //}
    }
    else
    {
        // Zero out external force/torque
        *m_rigidObject->getRigidBody()->m_force  = Vec3d(0.0, 0.0, 0.0);
        *m_rigidObject->getRigidBody()->m_torque = Vec3d(0.0, 0.0, 0.0);
        // Directly set position/rotation
        (*m_rigidObject->getRigidBody()->m_pos) = getPosition();
        (*m_rigidObject->getRigidBody()->m_orientation) = getOrientation();
    }

    this->postEvent(Event(RigidObjectController::modified()));
}

void
RigidObjectController::applyForces()
{
    if (!m_deviceClient->getButton(0))
    {
        // Apply force back to device
        if (m_rigidObject != nullptr && m_useSpring)
        {
            const Vec3d force = -fS * m_forceScaling;
            if (m_forceSmoothening)
            {
                m_forces.push_back(force);
                m_forceSum += force;
                if (static_cast<int>(m_forces.size()) > m_smoothingKernelSize)
                {
                    m_forceSum -= m_forces.front();
                    m_forces.pop_front();
                }
                const Vec3d avgForce = m_forceSum / m_forces.size();

                // Render only the spring force (not the other forces the body has)
                m_deviceClient->setForce(avgForce);
            }
            else
            {
                // Render only the spring force (not the other forces the body has)
                m_deviceClient->setForce(force);
            }
        }
    }
    else
    {
        m_deviceClient->setForce(Vec3d(0.0, 0.0, 0.0));
    }
}
}