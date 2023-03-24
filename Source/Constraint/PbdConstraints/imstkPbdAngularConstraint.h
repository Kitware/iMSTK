/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdAngularConstraint
///
/// \brief Applies rotational correction only
///
class PbdAngularConstraint : public PbdConstraint
{
public:
    PbdAngularConstraint(int numParticles) : PbdConstraint(numParticles) { }
    ~PbdAngularConstraint() override = default;

    IMSTK_TYPE_NAME(PbdAngularConstraint)

    ///
    /// \brief Update positions by projecting constraints.
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;
};

///
/// \class PbdAngularHingeConstraint
///
/// \brief Constraint a single oriented particle along an axes of rotation
/// Aligns oriented particles up axes to hinge axes
///
class PbdAngularHingeConstraint : public PbdAngularConstraint
{
public:
    PbdAngularHingeConstraint() : PbdAngularConstraint(1) { }
    ~PbdAngularHingeConstraint() override = default;

    void initConstraint(const PbdParticleId& pIdx0,
                        const Vec3d&         hingeAxes,
                        const double         compliance);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    Vec3d m_hingeAxes = Vec3d(1.0, 0.0, 0.0); // The axes it can rotate around
};

///
/// \class PbdAngularDistanceConstraint
///
/// \brief Constraints one orientation to be relative by a given angular distance/offset
/// to another orientation.
///
class PbdAngularDistanceConstraint : public PbdAngularConstraint
{
public:
    PbdAngularDistanceConstraint() : PbdAngularConstraint(2) { }
    ~PbdAngularDistanceConstraint() override = default;

    ///
    /// \brief Constraint p0 to match p1, zero rotational offset
    ///
    void initConstraint(const PbdParticleId& p0,
                        const PbdParticleId& p1,
                        const double         compliance);

    ///
    /// \brief Constrain p0 to match p1's orientation according to the
    /// the current rotational offset between them
    ///
    void initConstraintOffset(
        const PbdState&      bodies,
        const PbdParticleId& p0,
        const PbdParticleId& p1,
        const double         compliance);

    ///
    /// \brief Constrain p0 to match p1's orientation with specified rotational
    /// offset
    ///
    void initConstraintOffset(
        const PbdParticleId& p0,
        const PbdParticleId& p1,
        const Quatd          rotationalOffset,
        const double         compliance);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    Quatd m_offset = Quatd::Identity();
};
} // namespace imstk