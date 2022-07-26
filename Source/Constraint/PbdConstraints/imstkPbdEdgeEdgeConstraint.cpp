/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdEdgeEdgeConstraint.h"

namespace imstk
{
void
PbdEdgeEdgeConstraint::initConstraint(
    VertexMassPair ptA1, VertexMassPair ptA2,
    VertexMassPair ptB1, VertexMassPair ptB2,
    double stiffnessA, double stiffnessB)
{
    m_stiffnessA = stiffnessA;
    m_stiffnessB = stiffnessB;

    m_bodiesFirst[0] = ptA1;
    m_bodiesFirst[1] = ptA2;

    m_bodiesSecond[0] = ptB1;
    m_bodiesSecond[1] = ptB2;
}

bool
PbdEdgeEdgeConstraint::computeValueAndGradient(double&             c,
                                               std::vector<Vec3d>& dcdxA,
                                               std::vector<Vec3d>& dcdxB) const
{
    const Vec3d& x0 = *m_bodiesFirst[0].vertex;
    const Vec3d& x1 = *m_bodiesFirst[1].vertex;
    const Vec3d& x2 = *m_bodiesSecond[0].vertex;
    const Vec3d& x3 = *m_bodiesSecond[1].vertex;

    const double a  = (x3 - x2).dot(x1 - x0);
    const double b  = (x1 - x0).dot(x1 - x0);
    const double cc = (x0 - x2).dot(x1 - x0);
    const double d  = (x3 - x2).dot(x3 - x2);
    const double e  = a;
    const double f  = (x0 - x2).dot(x3 - x2);

    const double det = a * e - d * b;
    double       s   = 0.5;
    double       t   = 0.5;
    if (fabs(det) > 1e-12)
    {
        s = (cc * e - b * f) / det;
        t = (cc * d - a * f) / det;
        if (s < 0 || s > 1.0 || t < 0 || t > 1.0)
        {
            c = 0.0;
            return false;
        }
    }

    // Two closest points on the line segments
    const Vec3d P = x0 + t * (x1 - x0);
    const Vec3d Q = x2 + s * (x3 - x2);

    // Distance between nearest points on line segments
    Vec3d        n = (Q - P);
    const double l = n.norm();
    n /= l;

    if (l <= 0.0)
    {
        c = 0.0;
        return false;
    }

    dcdxA[0] = (1 - t) * n;
    dcdxA[1] = t * n;
    dcdxB[0] = -(1 - s) * n;
    dcdxB[1] = -s * n;

    c = l;

    return true;
}
} // namespace imstk