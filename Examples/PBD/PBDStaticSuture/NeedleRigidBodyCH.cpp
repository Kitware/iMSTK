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

#include "NeedleRigidBodyCH.h"
#include "NeedleObject.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "RbdPointToArcConstraint.h"

void
NeedleRigidBodyCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // Do it the normal way
    RigidBodyCH::handle(elementsA, elementsB);

    // If no collision, needle must be removed
    auto                         needleObj = std::dynamic_pointer_cast<NeedleObject>(getInputObjectA());
    NeedleObject::CollisionState state     = needleObj->getCollisionState();

    if (elementsA.size() != 0)
    {
        if (state == NeedleObject::CollisionState::INSERTED)
        {
            const Mat3d& arcBasis    = needleObj->getArcBasis();
            const Vec3d& arcCenter   = needleObj->getArcCenter();
            const double arcRadius   = needleObj->getArcRadius();
            const double arcBeginRad = needleObj->getBeginRad();
            const double arcEndRad   = needleObj->getEndRad();

            // Constrain along the axes, whilst allowing "pushing" of the contact point
            auto pointToArcConstraint = std::make_shared<RbdPointToArcConstraint>(
                needleObj->getRigidBody(),
                arcCenter,
                arcBeginRad, arcEndRad,
                arcRadius,
                arcBasis,
                m_initContactPt,
                m_beta);
            pointToArcConstraint->compute(needleObj->getRigidBodyModel2()->getTimeStep());
            needleObj->getRigidBodyModel2()->addConstraint(pointToArcConstraint);
        }
    }
    else
    {
        if (state == NeedleObject::CollisionState::INSERTED || state == NeedleObject::CollisionState::TOUCHING)
        {
            if (state == NeedleObject::CollisionState::INSERTED)
            {
                LOG(INFO) << "Unpuncture!";
            }
            needleObj->setCollisionState(NeedleObject::CollisionState::REMOVED);
        }
    }
}

void
NeedleRigidBodyCH::addConstraint(
    std::shared_ptr<RigidObject2> rbdObj,
    const Vec3d& contactPt, const Vec3d& contactNormal,
    const double contactDepth)
{
    auto obj = std::dynamic_pointer_cast<NeedleObject>(rbdObj);

    // If removed and we are here, we must now be touching
    if (obj->getCollisionState() == NeedleObject::CollisionState::REMOVED)
    {
        obj->setCollisionState(NeedleObject::CollisionState::TOUCHING);
    }

    // If touching we may test for insertion
    const Vec3d n = contactNormal.normalized();
    if (obj->getCollisionState() == NeedleObject::CollisionState::TOUCHING)
    {
        // Get all inwards force
        const double fN = std::max(-contactNormal.dot(obj->getRigidBody()->getForce()), 0.0);

        // If the normal force exceeds threshold the needle may insert
        if (fN > obj->getForceThreshold())
        {
            LOG(INFO) << "Puncture!";
            obj->setCollisionState(NeedleObject::CollisionState::INSERTED);

            // Record the initial contact point
            m_initOrientation = Quatd(rbdObj->getCollidingGeometry()->getRotation());
            m_initContactPt   = contactPt;
        }
    }

    // Only add contact normal constraint if not inserted
    NeedleObject::CollisionState state = obj->getCollisionState();
    if (state == NeedleObject::CollisionState::TOUCHING)
    {
        auto contactConstraint = std::make_shared<RbdContactConstraint>(
            rbdObj->getRigidBody(), nullptr,
            n, contactPt, contactDepth,
            m_beta,
            RbdConstraint::Side::A);
        contactConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
        rbdObj->getRigidBodyModel2()->addConstraint(contactConstraint);
    }
}