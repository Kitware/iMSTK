/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObjectController.h"
#include "imstkDeviceClient.h"
#include "imstkLogger.h"
#include "imstkPbdObject.h"

#include <Eigen/Eigenvalues>

namespace imstk
{
void
PbdObjectController::setControlledObject(std::shared_ptr<SceneObject> obj)
{
    m_pbdObject = std::dynamic_pointer_cast<PbdObject>(obj);
    CHECK(m_pbdObject != nullptr) << "Controlled object must be a PbdObject";
    CHECK(m_pbdObject->getPbdBody()->bodyType == PbdBody::Type::RIGID)
        << "PbdObjectController can only operate on pbd rigid bodies";
    SceneObjectController::setControlledObject(obj);
}

void
PbdObjectController::update(const double& dt)
{
    if (!updateTrackingData(dt))
    {
        LOG(WARNING) << "warning: could not update tracking info.";
        return;
    }

    if (m_pbdObject == nullptr)
    {
        return;
    }

    // Implementation partially from otaduy lin's paper eq14
    // "A Modular Haptic Rendering Algorithm for Stable and Transparent 6 - DOF Manipulation"
    if (m_deviceClient->getTrackingEnabled() && m_useSpring)
    {
        const Vec3d& currPos = (*m_pbdObject->getPbdBody()->vertices)[0];
        const Quatd& currOrientation     = (*m_pbdObject->getPbdBody()->orientations)[0];
        const Vec3d& currVelocity        = (*m_pbdObject->getPbdBody()->velocities)[0];
        const Vec3d& currAngularVelocity = (*m_pbdObject->getPbdBody()->angularVelocities)[0];
        Vec3d&       currForce  = m_pbdObject->getPbdBody()->externalForce;
        Vec3d&       currTorque = m_pbdObject->getPbdBody()->externalTorque;

        const Vec3d& devicePos = getPosition();
        const Quatd& deviceOrientation = getOrientation();
        const Vec3d& deviceOffset      = Vec3d(0.0, 0.0, 0.0);

        // If using critical damping automatically compute kd
        if (m_useCriticalDamping)
        {
            const double mass     = (*m_pbdObject->getPbdBody()->masses)[0];
            const double linearKs = m_linearKs.maxCoeff();
            m_linearKd = 2.0 * std::sqrt(mass * linearKs);

            const Mat3d inertia = (*m_pbdObject->getPbdBody()->inertias)[0];
            // Currently kd is not a 3d vector though it could be.
            // So here we make an approximation. Either:
            //  - Use one colums eigenvalue (maxCoeff)
            //  - cbrt(eigenvalue0*eigenvalue1*eigenvalue2). (det)
            // Both may behave weird on anistropic inertia tensors
            //const double inertiaScale = inertia.eigenvalues().real().maxCoeff();
            const double inertiaScale = std::cbrt(inertia.determinant());
            const double angularKs    = m_angularKs.maxCoeff();
            m_angularKd = 2.0 * std::sqrt(inertiaScale * angularKs);
        }

        // If kd > 2 * sqrt(mass * ks); The system is overdamped (may be intentional)
        // If kd < 2 * sqrt(mass * ks); The system is underdamped (never intended)

        // Uses non-relative force
        {
            // Compute force
            m_fS = m_linearKs.cwiseProduct(devicePos - currPos - deviceOffset);
            m_fD = m_linearKd * (-currVelocity - currAngularVelocity.cross(deviceOffset));
            Vec3d force = m_fS + m_fD;

            // Computer torque
            const Quatd dq = deviceOrientation * currOrientation.inverse();
            const Rotd  angleAxes = Rotd(dq);
            m_tS = deviceOffset.cross(force) + m_angularKs.cwiseProduct(angleAxes.axis() * angleAxes.angle());
            m_tD = m_angularKd * -currAngularVelocity;
            Vec3d torque = m_tS + m_tD;

            currForce  += force;
            currTorque += torque;
        }

        // Uses relative velocity
        //{
        //    const Vec3d& deviceVelocity = getVelocity();
        //    const Vec3d& deviceAngularVelocity = getAngularVelocity();
        //
        //    // Compute force
        //    m_fS = m_linearKs.cwiseProduct(devicePos - currPos - deviceOffset);
        //    m_fD = m_linearKd * (deviceVelocity - currVelocity - currAngularVelocity.cross(deviceOffset));
        //    Vec3d force = m_fS + m_fD;
        //
        //    // Compute torque
        //    const Quatd dq = deviceOrientation * currOrientation.inverse();
        //    const Rotd  angleAxes = Rotd(dq);
        //    m_tS = deviceOffset.cross(force) + m_angularKs.cwiseProduct(angleAxes.axis() * angleAxes.angle());
        //    m_tD = m_angularKd * (deviceAngularVelocity - currAngularVelocity);
        //    Vec3d torque = m_tS + m_tD;
        //
        //    currForce += force;
        //    currTorque += torque;
        //}
    }
    else
    {
        // Zero out external force/torque
        m_pbdObject->getPbdBody()->externalForce  = Vec3d(0.0, 0.0, 0.0);
        m_pbdObject->getPbdBody()->externalTorque = Vec3d(0.0, 0.0, 0.0);
        // Directly set position/rotation
        (*m_pbdObject->getPbdBody()->vertices)[0]     = getPosition();
        (*m_pbdObject->getPbdBody()->orientations)[0] = getOrientation();
    }

    applyForces();
}

void
PbdObjectController::applyForces()
{
    if (!m_deviceClient->getButton(0))
    {
        // Apply force back to device
        if (m_pbdObject != nullptr && m_useSpring)
        {
            const Vec3d force = -getDeviceForce();
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
} // namespace imstk