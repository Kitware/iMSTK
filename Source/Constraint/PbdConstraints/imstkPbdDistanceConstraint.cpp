/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdDistanceConstraint.h"

namespace  imstk
{
void
PbdDistanceConstraint::initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                                      const size_t& pIdx0,
                                      const size_t& pIdx1,
                                      const double k)
{
    m_vertexIds[0] = pIdx0;
    m_vertexIds[1] = pIdx1;
    m_stiffness    = k;
    m_compliance   = 1.0 / k;

    const Vec3d& p0 = initVertexPositions[pIdx0];
    const Vec3d& p1 = initVertexPositions[pIdx1];

    m_restLength = (p0 - p1).norm();
}

bool
PbdDistanceConstraint::computeValueAndGradient(
    const VecDataArray<double, 3>& currVertexPositions,
    double& c,
    std::vector<Vec3d>& dcdx) const
{
    const Vec3d& p0 = currVertexPositions[m_vertexIds[0]];
    const Vec3d& p1 = currVertexPositions[m_vertexIds[1]];

    dcdx[0] = p0 - p1;
    const double len = dcdx[0].norm();
    if (len == 0.0)
    {
        return false;
    }
    dcdx[0] /= len;
    dcdx[1]  = -dcdx[0];
    c        = len - m_restLength;

    return true;
}
} // namespace imstk
