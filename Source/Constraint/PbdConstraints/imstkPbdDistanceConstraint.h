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
/// \class PbdDistanceConstraint
///
/// \brief Distance constraints between two nodal points
///
class PbdDistanceConstraint : public PbdConstraint
{
public:
    PbdDistanceConstraint() : PbdConstraint()
    {
        m_vertexIds.resize(2);
        m_dcdx.resize(2);
    }

    ///
    /// \brief Initializes the distance constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx0,
                        const size_t& pIdx1,
                        const double k = 1e5);

    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

public:
    double m_restLength = 0.0; ///< Rest length between the nodes
};
} // namespace imstk