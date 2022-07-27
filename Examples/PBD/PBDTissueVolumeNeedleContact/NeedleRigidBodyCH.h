/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkRigidBodyCH.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "NeedleObject.h"

using namespace imstk;

///
/// \brief Surface collision disabled upon puncture
///
class NeedleRigidBodyCH : public RigidBodyCH
{
public:
    NeedleRigidBodyCH() = default;
    ~NeedleRigidBodyCH() override = default;

    IMSTK_TYPE_NAME(NeedleRigidBodyCH)

protected:
    ///
    /// \brief Add constraint for the rigid body given contact
    ///
    void addConstraint(
        std::shared_ptr<RigidObject2> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth) override
    {
        auto needleObj = std::dynamic_pointer_cast<NeedleObject>(rbdObj);

        if (needleObj->getCollisionState() == NeedleObject::CollisionState::REMOVED)
        {
            needleObj->setCollisionState(NeedleObject::CollisionState::TOUCHING);
        }

        // Only add contact normal constraint if not inserted
        if (needleObj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
        {
            const Vec3d n = contactNormal.normalized();

            auto contactConstraint = std::make_shared<RbdContactConstraint>(
                rbdObj->getRigidBody(), nullptr,
                n, contactPt, contactDepth,
                m_beta,
                RbdConstraint::Side::A);
            contactConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
            rbdObj->getRigidBodyModel2()->addConstraint(contactConstraint);
        }
    }
};