/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkSurfaceMesh.h"
#include "imstkMacros.h"

#include <unordered_map>

namespace imstk
{
class EmbeddingConstraint;
class Geometry;
class PbdCollisionSolver;
class PbdConstraint;
} // namespace imstk

using namespace imstk;

///
/// \class NeedleEmbeddedCH
///
/// \brief Implements PBD-RBD embedded tissue handling for when the
/// needle is embedded in the tissue
///
class NeedleEmbeddedCH : public CollisionHandling
{
public:
    ~NeedleEmbeddedCH() override = default;

    IMSTK_TYPE_NAME(NeedleEmbeddedCH)

public:
    std::shared_ptr<Geometry> getHandlingGeometryA() override;

public:
    void setFriction(const double friction) { m_friction = friction; }
    const double getFriction() const { return m_friction; }

protected:
    ///
    /// \brief Add embedding constraints based off contact data
    /// We need to add the constraint once and then update it later
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

private:
    // TriCell takes care of duplicate faces
    std::unordered_map<TriCell, std::shared_ptr<EmbeddingConstraint>> m_faceConstraints;

    std::vector<PbdConstraint*> m_solverConstraints; ///< List of PBD constraints

    //double m_restitution = 0.0; ///< Coefficient of restitution (1.0 = perfect elastic, 0.0 = inelastic)
    double m_friction = 0.001; ///< Coefficient of friction (1.0 = full frictional force, 0.0 = none)

public:
    std::vector<Vec3d> m_debugEmbeddingPoints; ///< Used for debug visualization
    std::vector<Vec3i> m_debugEmbeddedTriangles;
};