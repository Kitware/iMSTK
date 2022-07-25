/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
class LineMeshToLineMeshCCD;
class PbdCollisionSolver;
class PbdEdgeEdgeConstraint;
class PbdEdgeEdgeCCDConstraint;
class PbdObject;
class PbdPointEdgeConstraint;
class PbdPointPointConstraint;
class PbdPointTriangleConstraint;

struct MeshSide;
///
/// \class PbdCollisionHandling
///
/// \brief Implements PBD based collision handling
///
class PbdCollisionHandling : public CollisionHandling
{
public:
    PbdCollisionHandling();
    virtual ~PbdCollisionHandling() override;

    IMSTK_TYPE_NAME(PbdCollisionHandling)

public:
    ///
    /// \brief Return the solver of the collision constraints
    ///
    std::shared_ptr<PbdCollisionSolver> getCollisionSolver() const { return m_pbdCollisionSolver; }

    ///
    /// \brief Corrects for velocity (restitution and friction) after PBD is complete
    ///
    void correctVelocities();

    void setRestitution(const double restitution) { m_restitution = restitution; }
    const double getRestitution() const { return m_restitution; }

    void setFriction(const double friction) { m_friction = friction; }
    const double getFriction() const { return m_friction; }

protected:
    ///
    /// \brief Add collision constraints based off contact data
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Generates constraints in the pools between a mesh and non mesh
    ///
    void generateMeshNonMeshConstraints(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

    ///
    /// \brief Generates constraints in the pools between two meshes
    ///
    void generateMeshMeshConstraints(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB);

protected:
    ///
    /// \brief Add a vertex-triangle constraint
    ///
    virtual void addVTConstraint(
        VertexMassPair ptA,
        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Add an edge-edge constraint
    ///
    virtual void addEEConstraint(
        VertexMassPair ptA1, VertexMassPair ptA2,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Add an edge-edge CCD constraint
    ///
    virtual void addEECCDConstraint(
        VertexMassPair prev_ptA1, VertexMassPair prev_ptA2,
        VertexMassPair prev_ptB1, VertexMassPair prev_ptB2,
        VertexMassPair ptA1, VertexMassPair ptA2,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Add a point-edge constraint
    ///
    virtual void addPEConstraint(
        VertexMassPair ptA1,
        VertexMassPair ptB1, VertexMassPair ptB2,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief Add a point-point constraint
    ///
    virtual void addPPConstraint(
        VertexMassPair ptA, VertexMassPair ptB,
        double stiffnessA, double stiffnessB);

private:
    std::shared_ptr<PbdCollisionSolver> m_pbdCollisionSolver = nullptr;

    std::vector<PbdCollisionConstraint*> m_PBDConstraints; ///< List of PBD constraints

    // Lists important here as the memory locations should not change upon push_back
    // and the amount is not known a priori
    std::list<Vec3d> m_fixedPoints;
    std::list<Vec3d> m_fixedPointVelocities;

    std::vector<PbdEdgeEdgeConstraint*>      m_EEConstraintPool;
    std::vector<PbdEdgeEdgeCCDConstraint*>   m_EECCDConstraintPool;
    std::vector<PbdPointTriangleConstraint*> m_VTConstraintPool;
    std::vector<PbdPointEdgeConstraint*>     m_PEConstraintPool;
    std::vector<PbdPointPointConstraint*>    m_PPConstraintPool;

    double m_restitution = 0.0; ///< Coefficient of restitution (1.0 = perfect elastic, 0.0 = inelastic)
    double m_friction    = 0.0; ///< Coefficient of friction (1.0 = full frictional force, 0.0 = none)
};
} // namespace imstk
