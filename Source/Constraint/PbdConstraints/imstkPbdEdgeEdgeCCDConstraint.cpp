/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkEdgeEdgeCCDState.h"
#include "imstkPbdEdgeEdgeCCDConstraint.h"
#include "imstkLineMeshToLineMeshCCD.h"

namespace imstk
{
void
PbdEdgeEdgeCCDConstraint::initConstraint(
    Vec3d* prevPtA0, Vec3d* prevPtA1,
    Vec3d* prevPtB0, Vec3d* prevPtB1,
    const PbdParticleId& ptA0, const PbdParticleId& ptA1,
    const PbdParticleId& ptB0, const PbdParticleId& ptB1,
    double stiffnessA, double stiffnessB,
    int ccdSubsteps)
{
    m_prevEdgeA[0] = prevPtA0;
    m_prevEdgeA[1] = prevPtA1;
    m_prevEdgeB[0] = prevPtB0;
    m_prevEdgeB[1] = prevPtB1;

    m_particles[0] = ptA0;
    m_particles[1] = ptA1;
    m_particles[2] = ptB0;
    m_particles[3] = ptB1;

    m_stiffness[0] = stiffnessA;
    m_stiffness[1] = stiffnessB;
    m_ccdSubsteps  = ccdSubsteps;
}

void
PbdEdgeEdgeCCDConstraint::projectConstraint(PbdState& bodies,
                                            const double dt, const SolverType& type)
{
    // The CCD constraint takes many more substeps to ensure
    // convergence of the constraint
    for (int k = 0; k < m_ccdSubsteps; k++)
    {
        PbdCollisionConstraint::projectConstraint(bodies, dt / m_ccdSubsteps, type);
    }
}

bool
PbdEdgeEdgeCCDConstraint::computeValueAndGradient(PbdState& bodies,
                                                  double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& currPt0 = bodies.getPosition(m_particles[0]);
    const Vec3d& currPt1 = bodies.getPosition(m_particles[1]);
    const Vec3d& currPt2 = bodies.getPosition(m_particles[2]);
    const Vec3d& currPt3 = bodies.getPosition(m_particles[3]);

    const Vec3d& prevPt0 = *m_prevEdgeA[0];
    const Vec3d& prevPt1 = *m_prevEdgeA[1];
    const Vec3d& prevPt2 = *m_prevEdgeB[0];
    const Vec3d& prevPt3 = *m_prevEdgeB[1];

    EdgeEdgeCCDState prevState(prevPt0, prevPt1, prevPt2, prevPt3);
    EdgeEdgeCCDState currState(currPt0, currPt1, currPt2, currPt3);

    double    timeOfImpact  = 0.0;
    const int collisionType = EdgeEdgeCCDState::testCollision(prevState, currState, timeOfImpact);
    if (collisionType == 0)
    {
        c = 0.0;
        return false;
    }

    const double s  = currState.si();
    const double t  = currState.sj();
    const Vec3d  n0 = prevState.pi() - prevState.pj();
    const Vec3d  n1 = currState.pi() - currState.pj();

    Vec3d n = n1;
    // invert the normal if lines are crossing:
    bool crossing = false;
    if (n0.dot(n1) < 0)
    {
        n *= -1.0;
        crossing = true;
    }

    const double d = n.norm();
    if (d <= 0.0)
    {
        c = 0.0;
        return false;
    }
    n /= d;

    // keep the prev values static by assigning zero vector as solution gradient.
    // This can also be done by assigning invMass as zero for prev timestep vertices.
    dcdx[0] = (1 - s) * n;
    dcdx[1] = s * n;

    dcdx[2] = -(1 - t) * n;
    dcdx[3] = -t * n;

    if (crossing)
    {
        c = d + currState.thickness();
    }
    else
    {
        c = std::abs(d - currState.thickness());
    }

    return true;
}
} // namespace imstk