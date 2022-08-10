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

    ///
    /// \brief Update positions by projecting constraints.
    ///
    void projectConstraint(PbdState& bodies,
                           const double dt, const SolverType& type) override;
};
} // namespace imstk