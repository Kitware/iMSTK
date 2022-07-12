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
/// \class PbdDihedralConstraint
///
/// \brief Angular constraint between two triangular faces
///
class PbdDihedralConstraint : public PbdConstraint
{
public:
    PbdDihedralConstraint() : PbdConstraint(4) { }

    /**
      \brief initConstraint
             p3
            / | \
           /  |  \
          p0  |  p1
           \  |  /
            \ | /
              p2
      \param model
      \param pIdx1 index of p0
      \param pIdx2 index of p1
      \param pIdx3 index of p2
      \param pIdx4 index of p3
      \param k stiffness
    */
    void initConstraint(
        const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
        const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
        const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
        const double k);

    ///
    /// \brief Compute value and gradient of the constraint
    ///
    /// \param[in] currVertexPositions vector of current positions
    /// \param[inout] c constraint value
    ///
    bool computeValueAndGradient(PbdState& bodies,
                                 double& c, std::vector<Vec3d>& dcdx) override;

public:
    double m_restAngle = 0.0; ///< Rest angle
};
} // namespace imstk