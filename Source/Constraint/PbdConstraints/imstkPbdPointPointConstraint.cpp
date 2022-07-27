/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdPointPointConstraint.h"

namespace imstk
{
void
PbdPointPointConstraint::initConstraint(
    VertexMassPair ptA, VertexMassPair ptB,
    double stiffnessA, double stiffnessB)
{
    m_bodiesFirst[0]  = ptA;
    m_bodiesSecond[0] = ptB;

    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;
}

bool
PbdPointPointConstraint::computeValueAndGradient(double&             c,
                                                 std::vector<Vec3d>& dcdxA,
                                                 std::vector<Vec3d>& dcdxB) const
{
    // Current position during solve
    const Vec3d& x_a = *m_bodiesFirst[0].vertex;
    const Vec3d& x_b = *m_bodiesSecond[0].vertex;

    const Vec3d diff = x_b - x_a;
    c = diff.norm();

    if (c == 0.0)
    {
        return false;
    }

    const Vec3d n = diff / c;

    dcdxA[0] = n;
    dcdxB[0] = -n;

    return true;
}
} // namespace imstk