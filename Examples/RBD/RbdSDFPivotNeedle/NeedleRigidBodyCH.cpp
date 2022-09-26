/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleRigidBodyCH.h"
#include "imstkLineMesh.h"
#include "imstkNeedle.h"
#include "imstkPuncturable.h"
#include "imstkRbdContactConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkVecDataArray.h"
#include "RbdLineToPointRotationConstraint.h"
#include "RbdLineToPointTranslationConstraint.h"

using namespace imstk;

void
NeedleRigidBodyCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    // Do it the normal way
    RigidBodyCH::handle(elementsA, elementsB);

    // If no collision, needle must be removed
    // If using point based collision in an SDF you may want a differing unpuncturing constraint
    std::shared_ptr<CollidingObject> needleObj   = getInputObjectA();
    auto                             needle      = needleObj->getComponent<Needle>();
    std::shared_ptr<CollidingObject> tissueObj   = getInputObjectB();
    auto                             puncturable = tissueObj->getComponent<Puncturable>();
    const PunctureId                 punctureId  = getPunctureId(needle, puncturable);

    Puncture::State state = needle->getState(punctureId);
    if (elementsA.size() == 0)
    {
        if (state == Puncture::State::INSERTED)
        {
            needle->setState(punctureId, Puncture::State::REMOVED);
            LOG(INFO) << "Unpuncture!\n";
        }
        else if (state == Puncture::State::TOUCHING)
        {
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
    auto                             needle      = rbdObj->getComponent<Needle>();
    std::shared_ptr<CollidingObject> tissueObj   = getInputObjectB();
    auto                             puncturable = tissueObj->getComponent<Puncturable>();
    const PunctureId                 punctureId  = getPunctureId(needle, puncturable);

    // If the normal force exceeds threshold the needle may insert
    if (needle->getState(punctureId) == Puncture::State::REMOVED)
    {
        needle->setState(punctureId, Puncture::State::TOUCHING);
    }

    const Vec3d n = contactNormal.normalized();
    if (needle->getState(punctureId) == Puncture::State::TOUCHING)
    {
        // Get all inwards force
        const Vec3d  needleAxes = needle->getNeedleDirection();
        const double fN = std::max(needleAxes.dot(rbdObj->getRigidBody()->getForce()), 0.0);

        if (fN > 50.0)
        {
            LOG(INFO) << "Puncture!\n";
            needle->setState(punctureId, Puncture::State::INSERTED);
            puncturable->setPuncture(punctureId, needle->getPuncture(punctureId));

            // Record the axes to constrain too
            m_initNeedleAxes = needleAxes;
            m_initNeedleOrientation = Quatd::FromTwoVectors(Vec3d(0.0, -1.0, 0.0), needleAxes);
            m_initContactPt = contactPt;
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
    // Lock to the initial axes when the needle is inserted
    else if (state == Puncture::State::INSERTED)
    {
        auto   lineMesh = std::dynamic_pointer_cast<LineMesh>(rbdObj->getPhysicsGeometry());
        Vec3d* p = &(*lineMesh->getVertexPositions())[0];
        Vec3d* q = &(*lineMesh->getVertexPositions())[1];

        // This constraint solves for the translation to bring line p,q to m_initContactPt
        auto translationConstraint = std::make_shared<RbdLineToPointTranslationConstraint>(
            rbdObj->getRigidBody(), m_initContactPt, p, q, 0.1);
        translationConstraint->compute(rbdObj->getRigidBodyModel2()->getTimeStep());
        rbdObj->getRigidBodyModel2()->addConstraint(translationConstraint);

        // Bit of a cheat, but I parameterize the inertia by depth linearly. I use a large
        // jump after x=1 to really lock in the orientation after the needle has go so far
        double x = contactDepth / 0.02;
        if (x > 1.0)
        {
            x = 100.0;
        }
        rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() *
                                                   (10000.0 + x * 10000.0);
        rbdObj->getRigidBodyModel2()->updateMass();
    }
}