/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkEdgeEdgeCCDState.h"

namespace imstk
{
EdgeEdgeCCDState::EdgeEdgeCCDState(const Vec3d& i0, const Vec3d& i1, const Vec3d& j0, const Vec3d& j1)
    : xi(i0), xi1(i1), xj(j0), xj1(j1),
    ei(xi1 - xi), ej(xj1 - xj), w0(xj - xi),
    w(shortestDistanceVector())
{
    m_si = computeSi();
    m_sj = computeSj();
    m_pi = computePi();
    m_pj = computePj();
}

Vec3d
EdgeEdgeCCDState::computeWBar()
{
    /// Smallest vector formed by permuting between the points of the
    /// two edges (excluding the edges themselves).
    /// e.g. For edges (x1, x2) and (x3, x4) ..
    /// return the smallest vector among:
    /// (x1, x3) (x1, x4) (x2, x3), and (x2, x4).
    // modify s_i and s_j accordingly
    struct VertexPair
    {
        public:
            Vec3d m_diff;
            double m_si, m_sj;
            VertexPair(const Vec3d& diff, double si, double sj) : m_diff(diff), m_si(si), m_sj(sj)
            {}

            bool operator<(const VertexPair& A) const
            {
                return(this->m_diff.norm() < A.m_diff.norm());
            }
    };

    VertexPair p1(xj - xi, 0, 0);
    VertexPair p2(xj1 - xi, 1, 0);
    VertexPair p3(xj - xi1, 0, 1);
    VertexPair p4(xj1 - xi1, 1, 1);
    auto       result = std::min<VertexPair>({ p1, p2, p3, p4 });

    // CAUTION: we are overwriting m_si and m_sj values of EdgeEdgeCCDState here, assuming that
    // they are no longer needed. computeWBar() should only be called once it has
    // already been determined that an internal intersection doesn't exist.
    m_si = result.m_si;
    m_sj = result.m_sj;

    // Since m_si and m_sj have changed, recompute m_pi and m_pj
    m_pi = computePi();
    m_pj = computePj();

    return result.m_diff;
}

Vec3d
EdgeEdgeCCDState::computeWBar2() const
{
    Vec3d p1 = (xj - xi);
    Vec3d p2 = (xj1 - xi);
    Vec3d p3 = (xj - xi1);
    Vec3d p4 = (xj1 - xi1);
    return std::min<Vec3d>({ p1, p2, p3, p4 }, [](const Vec3d& a, const Vec3d& b) { return (a.norm() < b.norm()); });
}

int
EdgeEdgeCCDState::testCollision(const EdgeEdgeCCDState& prev, EdgeEdgeCCDState& curr, double& relativeTimeOfImpact)
{
    relativeTimeOfImpact = 0.0;
    const double tol = 0.01;
    const bool   externalIntersection = (curr.si() < 0 - tol || curr.si() > 1 + tol || curr.sj() < 0 - tol || curr.sj() > 1 + tol);

    Vec3d curr_wbar = curr.w;
    if (externalIntersection)
    {
        curr_wbar = curr.computeWBar2();
    }

    if (curr_wbar.norm() < prev.thickness() + prev.m_epsilon)
    {
        relativeTimeOfImpact = 1.0; // impact happens in current time step.
        return externalIntersection ? 2 : 1;
    }

    const bool crossedEachOther = (prev.w.dot(curr.w) < 0);
    if (crossedEachOther && !externalIntersection)
    {
        double m     = std::copysign(1.0, curr.w.dot(prev.w)); // get the sign of w_t dot w_{t+1}
        double denom = prev.w.norm() - m * curr.w.norm();

        if (denom > prev.m_epsilon) // avoid dividing by zero
        {
            relativeTimeOfImpact = prev.w.norm() / (prev.w.norm() - m * curr.w.norm());
        }
        return 3;
    }

    return 0;
}

double
EdgeEdgeCCDState::computeSi() const
{
    double ac_bb = denom();
    if (abs(ac_bb) < m_epsilon)
    {
        // Return something significantly outside the interval [0,1]
        // so that the downstream algorithm understands that it is not an internal intersection,
        // and directly checks for vertex-vertex closeness of the two segments.
        return -1.0;
    }

    // use of -1 multiplier is necessary because w0 is inverted.
    return -1.0 * (b() * e() - c() * d()) / ac_bb;
}

double
EdgeEdgeCCDState::computeSj() const
{
    double ac_bb = denom();
    if (abs(ac_bb) < m_epsilon)
    {
        if (b() < m_epsilon)
        {
            // Return something significantly outside the interval [0,1]
            // so that the downstream algorithm understands that it is not an internal intersection,
            // and directly checks for vertex-vertex closeness of the two segments.
            return -1.0;
        }
        else
        {
            return d() / b();
        }
    }

    // use of -1 multiplier is necessary because w0 is inverted.
    return -1.0 * (a() * e() - b() * d()) / ac_bb;
}

Vec3d
EdgeEdgeCCDState::shortestDistanceVector() const
{
    auto n = ei.cross(ej).normalized();
    auto d = w0.dot(n);
    return (d * n);
}
} // namespace imstk
