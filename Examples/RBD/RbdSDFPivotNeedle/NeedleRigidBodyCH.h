/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRigidBodyCH.h"
#include "imstkMacros.h"

using namespace imstk;

class NeedleRigidBodyCH : public RigidBodyCH
{
public:
    NeedleRigidBodyCH() = default;
    ~NeedleRigidBodyCH() override = default;

    IMSTK_TYPE_NAME(NeedleRigidBodyCH)

protected:
    ///
    /// \brief Handle the collision/contact data
    ///
    virtual void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Add constraint for the rigid body given contact
    ///
    void addConstraint(
        std::shared_ptr<RigidObject2> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth) override;

protected:
    Vec3d m_initContactPt  = Vec3d::Zero();
    Vec3d m_initNeedleAxes = Vec3d::Zero();
    Quatd m_initNeedleOrientation = Quatd::Identity();
};