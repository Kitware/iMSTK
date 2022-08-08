/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdAngularConstraint.h"

namespace imstk
{
///
/// \class PbdHingeJointConstraint
///
/// \brief Constraint a single oriented particle along an axes of rotation
/// Aligns oriented particles up axes to hinge axes
///
class PbdHingeJointConstraint : public PbdAngularConstraint
{
public:
    PbdHingeJointConstraint() : PbdAngularConstraint(1) { }
    ~PbdHingeJointConstraint() override = default;

    void initConstraint(const PbdParticleId& pIdx0,
                        const Vec3d&         hingeAxes,
                        const double         k);

    ///
    /// \brief Compute value and gradient of constraint function
    /// \param[inout] set of bodies involved in system
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

protected:
    Vec3d m_hingeAxes = Vec3d(1.0, 0.0, 0.0); // The axes with which you may rotate around
};
} // namespace imstk