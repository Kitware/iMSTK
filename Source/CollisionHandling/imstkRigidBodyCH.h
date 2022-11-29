/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkMacros.h"

#include <vector>

namespace imstk
{
class CollisionData;
class RigidObject2;

///
/// \class RigidBodyCH
///
/// \brief Creates rigid body contact and frictional constraints given
/// collision data then adds them to the rigid body model, if rigid
/// objects use differing models, one-way contacts are added to both.
/// If only one rigid object is given, one-ways are used.
///
class RigidBodyCH : public CollisionHandling
{
public:
    RigidBodyCH() = default;
    virtual ~RigidBodyCH() override = default;

    IMSTK_TYPE_NAME(RigidBodyCH)

public:
    void setInputRigidObjectA(std::shared_ptr<RigidObject2> rbdObjA);

    ///
    /// \brief Second input rigid object is optional
    ///
    void setInputRigidObjectB(std::shared_ptr<RigidObject2> rbdObjB);

    ///
    /// \brief Second input colliding object is optional
    ///
    // Do we want to rename this as setInputSceneObjectB ?
    void setInputCollidingObjectB(std::shared_ptr<Entity> colObjB);

    std::shared_ptr<RigidObject2> getRigidObjA();
    std::shared_ptr<RigidObject2> getRigidObjB();

public:
    ///
    /// \brief Baumgarte stabilization term, scales the constraint scalars
    /// The higher it is, the faster constraint violations are dealt with
    /// (bigger steps), but could introduce erroneous velocity
    ///
    void setBaumgarteStabilization(double beta) { m_beta = beta; }
    const double getBeta() const { return m_beta; }

    void setFriction(double frictionalCoefficient)
    {
        m_frictionalCoefficient = frictionalCoefficient;
        m_useFriction = (m_frictionalCoefficient != 0.0);
    }

    const double getFriction() const { return m_frictionalCoefficient; }

    void setUseFriction(bool useFriction) { m_useFriction = useFriction; }
    const double getUseFriction() const { return m_useFriction; }

protected:
    ///
    /// \brief Add rigid body constraints according to contacts
    ///
    virtual void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Handle rigid vs rigid two-way
    /// edge-edge, vertex-triangle, etc
    ///
    void handleRbdRbdTwoWay(
        std::shared_ptr<RigidObject2>        rbdObjA,
        std::shared_ptr<RigidObject2>        rbdObjB,
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

    ///
    /// \brief Handle rigid vs static one-way
    /// edge-edge, vertex-triangle, etc
    ///
    void handleRbdStaticOneWay(
        std::shared_ptr<RigidObject2>        rbdObj,
        std::shared_ptr<Entity>              colObj,
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

    ///
    /// \brief Add constraint for the rigid body given contact
    ///
    virtual void addConstraint(
        std::shared_ptr<RigidObject2> rbdObj,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth);

    ///
    /// \brief Add two-way constraint for the rigid bodies given contact
    ///
    virtual void addConstraint(
        std::shared_ptr<RigidObject2> rbdObjA,
        std::shared_ptr<RigidObject2> rbdObjB,
        const Vec3d& contactPt, const Vec3d& contactNormal,
        const double contactDepth);

protected:
    double m_beta = 0.0;
    double m_frictionalCoefficient = 2.0;
    bool   m_useFriction = false;
};
} // namespace imstk