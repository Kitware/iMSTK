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

#pragma once

#include "imstkMath.h"

#include <memory>

namespace imstk
{
class PointSet;

///
/// \brief Serves as a handle to the body
///
struct RigidBody
{
    public:
        double m_mass = 1.0;
        Mat3d m_intertiaTensor = Mat3d::Identity();
        Vec3d m_initPos = Vec3d(0.0, 0.0, 0.0);
        Quatd m_initOrientation     = Quatd(1.0, 0.0, 0.0, 0.0);
        Vec3d m_initVelocity        = Vec3d(0.0, 0.0, 0.0);
        Vec3d m_initAngularVelocity = Vec3d(0.0, 0.0, 0.0);
        Vec3d m_initForce  = Vec3d(0.0, 0.0, 0.0);
        Vec3d m_initTorque = Vec3d(0.0, 0.0, 0.0);
        bool m_isStatic    = false;

        Vec3d m_prevForce = Vec3d(0.0, 0.0, 0.0);

        Vec3d* m_pos = nullptr;
        Quatd* m_orientation     = nullptr;
        Vec3d* m_velocity        = nullptr;
        Vec3d* m_angularVelocity = nullptr;
        Vec3d* m_force  = nullptr;
        Vec3d* m_torque = nullptr;

    public:
        const Vec3d& getPosition() const { return *m_pos; }
        const Quatd& getOrientation() const { return *m_orientation; }
        const Vec3d& getVelocity() const { return *m_velocity; }
        const Vec3d& getAngularVelocity() const { return *m_angularVelocity; }
        const Vec3d& getForce() const { return *m_force; }
        const Vec3d& getTorque() const { return *m_torque; }

        const double getMass() const { return m_mass; }
        void setMass(const double mass) { m_mass = mass; }

        const Mat3d& getIntertiaTensor() const { return m_intertiaTensor; }
        void setInertiaTensor(const Mat3d& inertiaTensor) { m_intertiaTensor = inertiaTensor; }

        const Vec3d& getInitPos() const { return m_initPos; }
        void setInitPos(const Vec3d& pos) { m_initPos = pos; }

        const Quatd& getInitOrientation() const { return m_initOrientation; }
        void setInitOrientation(const Quatd& orientation) { m_initOrientation = orientation; }

        const Vec3d& getInitVelocity() const { return m_initVelocity; }
        void setInitVelocity(const Vec3d& velocity) { m_initVelocity = velocity; }

        const Vec3d& getInitAngularVelocity() const { return m_initAngularVelocity; }
        void setInitAngularVelocity(const Vec3d& angularVelocity) { m_initAngularVelocity = angularVelocity; }

        const Vec3d& getInitForce() const { return m_initForce; }
        void setInitForce(const Vec3d& force) { m_initForce = force; }

        const Vec3d& getInitTorque() const { return m_initTorque; }
        void setInitTorque(const Vec3d& torque) { m_initTorque = torque; }

        ///
        /// \brief Convience function to set the inertia tensor based off provided geometry
        /// assuming uniform mass at each point.
        ///
        void setInertiaFromPointSet(std::shared_ptr<PointSet> pointset, const double scale = 1.0, const bool useBoundingBoxOrigin = true);
};

///
/// \class RbdConstraint
///
/// \brief Abstract class for rigid body constraints
/// \todo: consider removing static flags and using sides in the interaction
///
class RbdConstraint
{
public:
    enum class Side
    {
        A,
        B,
        AB
    };

protected:
    RbdConstraint(std::shared_ptr<RigidBody> rbd1,
                  std::shared_ptr<RigidBody> rbd2, const Side side) :
        m_obj1(rbd1), m_obj2(rbd2), m_side(side)
    {
    }

public:
    virtual ~RbdConstraint() = default;

public:
    ///
    /// \brief Compute constraint jacobian
    ///
    virtual void compute(double dt) = 0;

public:
    // Jacobian
    Eigen::Matrix<double, 3, 4> J = Eigen::Matrix<double, 3, 4>::Zero();
    double vu = 0.0;
    // Range of the constraint force (for projection step during solve)
    // by default (0, inf) so bodies may only be pushed apart
    double range[2] = { 0.0, std::numeric_limits<double>::max() };

    // Objects involved
    std::shared_ptr<RigidBody> m_obj1 = nullptr;
    std::shared_ptr<RigidBody> m_obj2 = nullptr;

    // Which object to solve for
    Side m_side = Side::AB;
};
}