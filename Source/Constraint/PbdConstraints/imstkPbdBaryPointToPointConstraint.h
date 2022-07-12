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

    double getRestLength() const { return m_restLength; }
    void setRestLength(const double length) { m_restLength = length; }

    Vec3d computeInterpolantDifference(PbdState& bodies) const;

    ///
    /// \brief initialize constraint with current distance between
    /// the points as the resting length
    /// \param points of cell a
    /// \param barycentric weights for the point in cell a
    /// \param points of cell b
    /// \param barycentric weights for the point in cell b
    /// \param Stiffness which to resolve a
    /// \param stiffness which to resolve b
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
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    double m_restLength = 0.0;
    // Bary weights A and B ordered in sequence
    std::vector<double> m_weights;
};
} // namespace imstk