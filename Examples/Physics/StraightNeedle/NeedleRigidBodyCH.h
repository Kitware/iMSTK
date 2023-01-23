/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkNeedle.h"
#include "imstkPuncturable.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdObject.h"

using namespace imstk;

class NeedleRigidBodyCH : public PbdCollisionHandling
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
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override
    {
        std::shared_ptr<Entity> needleObj = getInputObjectA();
        m_needle = needleObj->getComponent<Needle>();
        std::shared_ptr<Entity> tissueObj = getInputObjectB();
        m_puncturable = tissueObj->getComponent<Puncturable>();

        // Do it the normal way
        PbdCollisionHandling::handle(elementsA, elementsB);

        // If no collision, needle must be removed
        if (elementsA.size() == 0)
        {
            m_needle->setState(getPunctureId(m_needle, m_puncturable), Puncture::State::REMOVED);
        }
    }

///
/// \brief Add constraint for the rigid body given contact
///
/*
    // \todo: transfer the puncturing logic to a behavior or equivalent.
    // It shouldn't be implemented as a override of a CollisionHandler.
    // The role of a CollisionHandler should be solely to generate constraints.
    void addConstraint(
        std::shared_ptr<PbdObject> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth) override
    {
        // If the normal force exceeds threshold the needle may insert
        const PunctureId punctureId = getPunctureId(m_needle, m_puncturable);

        const Vec3d n = contactNormal.normalized();
        if (m_needle->getState(punctureId) == Puncture::State::TOUCHING)
        {
            // Get all inwards force
            const Vec3d  needleAxes = m_needle->getNeedleDirection();
            const double fN = std::max(needleAxes.dot(rbdObj->getRigidBody()->getForce()), 0.0);

            if (fN > m_needleForceThreshold)
            {
                LOG(INFO) << "Puncture!\n";
                m_needle->setState(punctureId, Puncture::State::INSERTED);

                // Record the axes to constrain too
                m_initNeedleAxes = needleAxes;
                m_initNeedleOrientation = Quatd::FromTwoVectors(Vec3d(0.0, -1.0, 0.0), needleAxes);
                m_initContactPt = contactPt;
            }
        }

        // Only add contact normal constraint if not inserted
        if (m_needle->getState(punctureId) == Puncture::State::TOUCHING)
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
        else if (m_needle->getState(punctureId) == Puncture::State::INSERTED)
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
    */

protected:
    double m_needleForceThreshold = 250.0; ///< When needle body exceeds this it inserts

    std::shared_ptr<Needle>      m_needle;
    std::shared_ptr<Puncturable> m_puncturable;
    Vec3d m_initContactPt  = Vec3d::Zero();
    Vec3d m_initNeedleAxes = Vec3d::Zero();
    Quatd m_initNeedleOrientation = Quatd::Identity();
};