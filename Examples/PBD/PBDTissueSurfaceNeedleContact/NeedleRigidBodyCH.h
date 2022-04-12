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

#include "imstkMacros.h"
#include "imstkRigidBodyCH.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"

#include "NeedleObject.h"
#include "RbdAxesLockingConstraint.h"
#include "RbdAngularLockingConstraint.h"

using namespace imstk;

class NeedleRigidBodyCH : public RigidBodyCH
{
public:
    NeedleRigidBodyCH() = default;
    ~NeedleRigidBodyCH() override = default;

    IMSTK_TYPE_NAME(NeedleRigidBodyCH)

    void setNeedleForceThreshold(double needleForceThreshold) { m_needleForceThreshold = needleForceThreshold; }
    double getNeedleForceThrehsold() const { return m_needleForceThreshold; }

protected:
    ///
    /// \brief Handle the collision/contact data
    ///
    virtual void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override
    {
        // Do it the normal way
        RigidBodyCH::handle(elementsA, elementsB);

        // If no collision, needle must be removed
        if (elementsA.size() == 0)
        {
            auto needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectA());
            needleObj->setCollisionState(NeedleObject::CollisionState::REMOVED);
        }
    }

    ///
    /// \brief Add constraint for the rigid body given contact
    ///
    void addConstraint(
        std::shared_ptr<RigidObject2> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(rbdObj);

        // If the normal force exceeds threshold the needle may insert
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::REMOVED)
        {
            needleObj->setCollisionState(NeedleObject::CollisionState::TOUCHING);
        }

        const Vec3d n = contactNormal.normalized();
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            // Get all inwards force
            const Vec3d  needleAxes = needleObj->getAxes();
            const double fN = std::max(needleAxes.dot(needleObj->getRigidBody()->getForce()), 0.0);

            if (fN > m_needleForceThreshold)
            {
                LOG(INFO) << "Puncture!\n";
                needleObj->setCollisionState(NeedleObject::CollisionState::INSERTED);

                // Record the axes to constrain too
                m_initNeedleAxes = needleAxes;
                m_initNeedleOrientation = Quatd(needleObj->getCollidingGeometry()->getRotation());
                m_initContactPt = contactPt;
            }
        }

        // Only add contact normal constraint if not inserted
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            auto contactConstraint = std::make_shared<RbdContactConstraint>(
                rbdObj->getRigidBody(), nullptr,
                n, contactPt, contactDepth,
                m_beta,
                RbdConstraint::Side::A);
            contactConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(contactConstraint);
        }
        // Lock to the initial axes when the needle is inserted
        else if (needleObj->getCollisionState() == NeedleObject::CollisionState::INSERTED)
        {
            auto needleLockConstraint = std::make_shared<RbdAxesLockingConstraint>(
                rbdObj->getRigidBody(),
                m_initContactPt, m_initNeedleAxes,
                0.05);
            needleLockConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(needleLockConstraint);

            auto needleLockConstraint2 = std::make_shared<RbdAngularLockingConstraint>(
                rbdObj->getRigidBody(), m_initNeedleOrientation, 0.05);
            needleLockConstraint2->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(needleLockConstraint2);
        }
    }

protected:
    double m_needleForceThreshold = 250.0; ///< When needle body exceeds this it inserts

    Vec3d m_initContactPt  = Vec3d::Zero();
    Vec3d m_initNeedleAxes = Vec3d::Zero();
    Quatd m_initNeedleOrientation = Quatd::Identity();
};