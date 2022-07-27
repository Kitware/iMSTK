/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdAreaConstraint.h"

namespace imstk
{
void
PbdAreaConstraint::initConstraint(const VecDataArray<double, 3>& initVertexPositions,
                                  const size_t& pIdx0,
                                  const size_t& pIdx1,
                                  const size_t& pIdx2,
                                  const double k)
{
    m_vertexIds[0] = pIdx0;
    m_vertexIds[1] = pIdx1;
    m_vertexIds[2] = pIdx2;

    this->m_stiffness  = k;
    this->m_compliance = 1.0 / k;

    const Vec3d& p0 = initVertexPositions[pIdx0];
    const Vec3d& p1 = initVertexPositions[pIdx1];
    const Vec3d& p2 = initVertexPositions[pIdx2];

    m_restArea = 0.5 * (p1 - p0).cross(p2 - p0).norm();
}

bool
PbdAreaConstraint::computeValueAndGradient(
    const VecDataArray<double, 3>& currVertexPositions,
    double& c,
    std::vector<Vec3d>& dcdx) const
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];
    const auto i3 = m_vertexIds[2];

    const Vec3d& p0 = currVertexPositions[i1];
    const Vec3d& p1 = currVertexPositions[i2];
    const Vec3d& p2 = currVertexPositions[i3];

    const Vec3d e0 = p0 - p1;
    const Vec3d e1 = p1 - p2;
    const Vec3d e2 = p2 - p0;

    Vec3d n = e0.cross(e1);
    c = 0.5 * n.norm();

    if (c < m_epsilon)
    {
        return false;
    }

    n /= 2 * c;
    c -= m_restArea;

    dcdx[0] = e1.cross(n);
    dcdx[1] = e2.cross(n);
    dcdx[2] = e0.cross(n);

    return true;
}
} // namespace imstk
