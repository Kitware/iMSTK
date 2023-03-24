/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdBaryPointToPointConstraint
///
/// \brief Constrains two points from two separate cells/elements given via
/// barycentric coordinates to be coincident
///
/// Such constraint may be used for grasping (grabbing points on elements,
/// grabbing points with other points), stitching (constraining two
/// points from separate elements together)
///
class PbdBaryPointToPointConstraint : public PbdCollisionConstraint
{
public:
    PbdBaryPointToPointConstraint() : PbdCollisionConstraint(0, 0) { }
    ~PbdBaryPointToPointConstraint() override = default;

    IMSTK_TYPE_NAME(PbdBaryPointToPointConstraint)

    double getRestLength() const { return m_restLength; }
    void setRestLength(const double length) { m_restLength = length; }

    ///
    /// \param bodies of the simulation (contains state data)
    ///
    Vec3d computeInterpolantDifference(PbdState& bodies) const;

    ///
    /// \brief initialize constraint with current distance between
    /// the points as the resting length
    /// \param bodies Bodies of the simulation
    /// \param ptIdsA points of cell a
    /// \param weightsA barycentric weights for the point in cell a
    /// \param ptIdsB points of cell b
    /// \param weightsB barycentric weights for the point in cell b
    /// \param stiffnessB Stiffness which to resolve a
    /// \param stiffnessA stiffness which to resolve b
    ///
    void initConstraintToRest(
        PbdState& bodies,
        const std::vector<PbdParticleId>& ptIdsA,
        const std::vector<double>& weightsA,
        const std::vector<PbdParticleId>& ptIdsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB);

    ///
    /// \brief Initialize the constraint
    ///
    void initConstraint(
        const std::vector<PbdParticleId>& ptIdsA,
        const std::vector<double>& weightsA,
        const std::vector<PbdParticleId>& ptIdsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB,
        const double restLength = 0.0);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] bodies set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

    ///
    /// \brief Return the rest configuration for the constraint
    ///
    double getRestValue() const { return m_restLength; }

protected:
    double m_restLength = 0.0;
    // Bary weights A and B ordered in sequence
    std::vector<double> m_weights;
};
} // namespace imstk