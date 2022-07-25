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
/// \class PbdVolumeConstraint
///
/// \brief Volume constraint for tetrahedral element
///
class PbdVolumeConstraint : public PbdConstraint
{
public:
    PbdVolumeConstraint()
    {
        m_vertexIds.resize(4);
        m_dcdx.resize(4);
    }

    ///
    /// \brief Initializes the volume constraint
    ///
    void initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                        const size_t& pIdx1, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4,
                        const double k = 2.0);

    ///
    /// \brief Compute the value and gradient of constraint
    ///
    bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPosition,
        double& c,
        std::vector<Vec3d>& dcdx) const override;

protected:
    double m_restVolume = 0.0; ///< Rest volume
};
} // namespace imstk