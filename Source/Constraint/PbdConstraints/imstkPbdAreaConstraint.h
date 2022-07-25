/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
////
/// \class PbdAreaConstraint
///
/// \brief Area constraint for triangular face
///
class PbdAreaConstraint : public PbdConstraint
{
public:
    PbdAreaConstraint() : PbdConstraint()
    {
        m_vertexIds.resize(3);
        m_dcdx.resize(3);
    }

    ///
    /// \brief Initializes the area constraint
    ///
    void initConstraint(
        const VecDataArray<double, 3>& initVertexPositions,
        const size_t& pIdx1, const size_t& pIdx2, const size_t& pIdx3,
        const double k = 2.5);

    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

public:
    double m_restArea = 0.0;  ///< Area at the rest position
};
} // namespace imstk