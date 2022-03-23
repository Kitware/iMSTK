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