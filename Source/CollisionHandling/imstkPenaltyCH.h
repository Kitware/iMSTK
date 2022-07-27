/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkMacros.h"

namespace imstk
{
class CollidingObject;
class CollisionData;
class FeDeformableObject;
class RigidObject2;

///
/// \class PenaltyCH
///
/// \brief Implements penalty collision handling between Fem and Rigid models
///
class PenaltyCH : public CollisionHandling
{
public:
    PenaltyCH() = default;
    virtual ~PenaltyCH() override = default;

    IMSTK_TYPE_NAME(PenaltyCH)

public:
    void setInputFeObject(std::shared_ptr<FeDeformableObject> feObj);
    void setInputRbdObject(std::shared_ptr<RigidObject2> rbdObj);

    std::shared_ptr<FeDeformableObject> getInputFeObject();
    std::shared_ptr<RigidObject2> getInputRbdObject();

public:
    ///
    /// \brief Set the contact stiffness
    ///
    void setContactStiffness(const double stiffness) { m_stiffness = stiffness; }

    ///
    /// \brief Set the contact velocity damping
    ///
    void setContactVelocityDamping(const double damping) { m_damping = damping; }

protected:
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Given the collision data, applies contact as external force
    /// to the rigid body (onyl supports PointDirection contacts)
    ///
    void computeContactForcesAnalyticRigid(
        const std::vector<CollisionElement>& elements,
        std::shared_ptr<RigidObject2>        analyticObj);

    ///
    /// \brief Given the collision data, applies nodal forces in the FEM model
    ///
    void computeContactForcesDiscreteDeformable(
        const std::vector<CollisionElement>& elements,
        std::shared_ptr<FeDeformableObject>  deformableObj);

protected:
    double m_stiffness = 5.0e5; ///< Stiffness of contact
    double m_damping   = 0.5;   ///< Damping of the contact
};
} // namespace imstk