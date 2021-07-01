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

#include "imstkRigidBodyCH.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "NeedleObject.h"
#include "RbdLinearNeedleLockingConstraint.h"
#include "RbdAngularNeedleLockingConstraint.h"

using namespace imstk;

class NeedleRigidBodyCH : public RigidBodyCH
{
public:
    NeedleRigidBodyCH() = default;
    ~NeedleRigidBodyCH() override = default;

    virtual const std::string getTypeName() const override { return "NeedleRigidBodyCH"; }

public:
    void setNeedleForceThreshold(double needleForceThreshold) { m_needleForceThreshold = needleForceThreshold; }
    double getNeedleForceThrehsold() const { return m_needleForceThreshold; }

protected:
    ///
    /// \brief Handle the collision/contact data
    ///
    virtual void handle(
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB) override
    {
        // Do it the normal way
        RigidBodyCH::handle(elementsA, elementsB);

        // If no collision, needle must be removed
        if (elementsA.getSize() == 0)
        {
            auto needleObject = std::dynamic_pointer_cast<NeedleObject>(getInputObjectA());
            needleObject->setInserted(false);
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
        auto        needleObject = std::dynamic_pointer_cast<NeedleObject>(rbdObj);
        const Vec3d n = contactNormal.normalized();

        // Get all inwards force
        const Vec3d  needleAxes = needleObject->getNeedleAxes();
        const double fN = std::max(needleAxes.dot(needleObject->getRigidBody()->getForce()), 0.0);

        // If the normal force exceeds 150 the needle may insert
        if (fN > m_needleForceThreshold && !needleObject->getInserted())
        {
            LOG(INFO) << "Puncture!\n";
            needleObject->setInserted(true);

            // Record the axes to constrain too
            m_initNeedleAxes = needleAxes;
            m_initNeedleOrientation = Quatd(needleObject->getCollidingGeometry()->getRotation());
            m_initContactPt = contactPt;
        }

        // Only add contact normal constraint if not inserted
        if (!needleObject->getInserted())
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
        else
        {
            auto needleLockConstraint = std::make_shared<RbdLinearNeedleLockingConstraint>(
                rbdObj->getRigidBody(),
                m_initContactPt, m_initNeedleAxes,
                0.05);
            needleLockConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(needleLockConstraint);

            auto needleLockConstraint2 = std::make_shared<RbdAngularNeedleLockingConstraint>(
                rbdObj->getRigidBody(), m_initNeedleOrientation, 0.05);
            needleLockConstraint2->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(needleLockConstraint2);
        }

        /*if (m_useFriction)
        {
            std::shared_ptr<RbdFrictionConstraint> frictionConstraint =
                std::make_shared<RbdFrictionConstraint>(
                    rbdObj->getRigidBody(), nullptr,
                    contactPt, contactNormal.normalized(), contactDepth,
                    m_frictionalCoefficient,
                    RbdConstraint::Side::A);
            frictionConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(frictionConstraint);
        }*/
    }

protected:
    double m_needleForceThreshold = 250.0; ///> When needle body exceeds this it inserts

    Vec3d m_initContactPt  = Vec3d::Zero();
    Vec3d m_initNeedleAxes = Vec3d::Zero();
    Quatd m_initNeedleOrientation = Quatd::Identity();
};