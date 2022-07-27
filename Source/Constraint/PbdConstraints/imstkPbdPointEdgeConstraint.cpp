/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdPointEdgeConstraint.h"

namespace imstk
{
void
PbdPointEdgeConstraint::initConstraint(
    VertexMassPair ptA1,
    VertexMassPair ptB1, VertexMassPair ptB2,
    double stiffnessA, double stiffnessB)
{
    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;

    m_bodiesFirst[0] = ptA1;

    m_bodiesSecond[0] = ptB1;
    m_bodiesSecond[1] = ptB2;
}

bool
PbdPointEdgeConstraint::computeValueAndGradient(double&             cc,
                                                std::vector<Vec3d>& dcdxA,
                                                std::vector<Vec3d>& dcdxB) const
{
    // Just project x0 onto x3-x2. Get the normal component for distance to line
    const Vec3d& x0 = *m_bodiesFirst[0].vertex;

    const Vec3d& x2 = *m_bodiesSecond[0].vertex;
    const Vec3d& x3 = *m_bodiesSecond[1].vertex;

    const Vec3d  ab     = x3 - x2;
    const double length = ab.norm();
    if (length == 0.0)
    {
        // There is no distance between the edge, can't do anything
        cc = 0.0;
        return false;
    }
    const Vec3d dir1 = ab / length;

    // Project onto the line
    const Vec3d  diff = x0 - x2;
    const double p    = dir1.dot(diff);
    if (p < 0.0 || p > length)
    {
        cc = 0.0;
        return false;
    }
    // Remove tangent component to get normal
    const Vec3d  diff1 = diff - p * dir1;
    const double l     = diff1.norm();
    if (l == 0.0)
    {
        // The point is on the line
        cc = 0.0;
        return false;
    }
    const Vec3d  n = diff1 / l;
    const double u = p / length;

    dcdxA[0] = -n;
    dcdxB[0] = (1.0 - u) * n;
    dcdxB[1] = u * n;

    cc = l;

    return true;
}
} // namespace imstk