/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleRigidBodyCH.h"
#include "imstkArcNeedle.h"
#include "imstkPuncturable.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "RbdPointToArcConstraint.h"

void
NeedleRigidBodyCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // Do it the normal way
    RigidBodyCH::handle(elementsA, elementsB);

    // If no collision, needle must be removed
    auto needleObj = std::dynamic_pointer_cast<RigidObject2>(getInputObjectA());
    auto needle    = needleObj->getComponent<ArcNeedle>();

    std::shared_ptr<CollidingObject> tissueObj   = getInputObjectB();
    auto                             puncturable = tissueObj->getComponent<Puncturable>();

    const PunctureId      punctureId = getPunctureId(needle, puncturable);
    const Puncture::State state      = needle->getState(punctureId);

    if (elementsA.size() != 0)
    {
        if (state == Puncture::State::INSERTED)
        {
            const Mat3d& rot = needleObj->getRigidBody()->getOrientation().toRotationMatrix();
            const Vec3d& pos = needleObj->getRigidBody()->getPosition();

            const Mat3d& arcBasis    = rot * needle->getArcBasis();
            const Vec3d& arcCenter   = pos + rot * needle->getArcCenter();
            const double arcRadius   = needle->getArcRadius();
            const double arcBeginRad = needle->getBeginRad();
            const double arcEndRad   = needle->getEndRad();

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
        if (state == Puncture::State::INSERTED || state == Puncture::State::TOUCHING)
        {
            if (state == Puncture::State::INSERTED)
            {
                LOG(INFO) << "Unpuncture!";
            }
            needle->setState(punctureId, Puncture::State::REMOVED);
        }
    }
}

void
NeedleRigidBodyCH::addConstraint(
    std::shared_ptr<RigidObject2> rbdObj,
    const Vec3d& contactPt, const Vec3d& contactNormal,
    const double contactDepth)
{
    // If no collision, needle must be removed
    auto needleObj = std::dynamic_pointer_cast<RigidObject2>(getInputObjectA());
    auto needle    = needleObj->getComponent<ArcNeedle>();

    std::shared_ptr<CollidingObject> tissueObj   = getInputObjectB();
    auto                             puncturable = tissueObj->getComponent<Puncturable>();
    const PunctureId                 punctureId  = getPunctureId(needle, puncturable);

    // If removed and we are here, we must now be touching
    if (needle->getState(punctureId) == Puncture::State::REMOVED)
    {
        needle->setState(punctureId, Puncture::State::TOUCHING);
    }

    // If touching we may test for insertion
    const Vec3d n = contactNormal.normalized();
    if (needle->getState(punctureId) == Puncture::State::TOUCHING)
    {
        // Get all inwards force
        const double fN = std::max(-contactNormal.dot(rbdObj->getRigidBody()->getForce()), 0.0);

        // If the normal force exceeds threshold the needle may insert
        if (fN > m_forceThreshold)
        {
            LOG(INFO) << "Puncture!";
            needle->setState(punctureId, Puncture::State::INSERTED);
            puncturable->setPuncture(punctureId, needle->getPuncture(punctureId));

            // Record the initial contact point
            m_initOrientation = Quatd(rbdObj->getCollidingGeometry()->getRotation());
            m_initContactPt   = contactPt;
        }
    }

    // Only add contact normal constraint if not inserted
    Puncture::State state = needle->getState(punctureId);
    if (state == Puncture::State::TOUCHING)
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