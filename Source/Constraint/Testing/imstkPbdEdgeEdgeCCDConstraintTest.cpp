/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkEdgeEdgeCCDState.h"
#include "imstkLineMeshToLineMeshCCD.h"
#include "imstkPbdConstraintTest.h"
#include "imstkPbdEdgeEdgeCCDConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;
static const double edgeEdgeCCDTolerance = 1e-6;

///
/// \brief Test that two colliding line segments resolve.
///
TEST_F(PbdConstraintTest, EdgeEdgeCCDConstraint_TestCollision1)
{
    setNumParticles(8);

    m_invMasses.fill(0.0);

    // We place both current and previous vertices in one array
    // In imstk previous vertices are put in a virtual buffer of particles

    // lineMeshA_prev
    m_vertices[0] = Vec3d(0.00, 0.00, -0.01);
    m_vertices[1] = Vec3d(0.00, 0.00, 0.01);

    // lineMeshA_curr
    m_vertices[2] = Vec3d(0.00, 0.00, -0.01);
    m_vertices[3] = Vec3d(0.00, 0.00, 0.01);

    // lineMeshB_prev
    m_vertices[4] = Vec3d(-0.01, 0.01, 0.00);
    m_vertices[5] = Vec3d(0.01, 0.01, 0.00);

    // lineMeshB_curr
    m_vertices[6]  = Vec3d(-0.01, -0.01, 0.00);
    m_invMasses[6] = 1.0;
    m_vertices[7]  = Vec3d(0.01, -0.01, 0.00);
    m_invMasses[7] = 1.0;

    PbdEdgeEdgeCCDConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        &m_vertices[0], &m_vertices[1], // prevA
        &m_vertices[4], &m_vertices[5], // prevB
        { 0, 2 }, { 0, 3 },             // currA
        { 0, 6 }, { 0, 7 },             // currB
        0.1, 0.1);

    // 100 iterations is a large number, but the test is simply to verify convergence.
    // In practice, fewer iterations can work.
    for (int i = 0; i < 100; i++)
    {
        solve(0.05, PbdConstraint::SolverType::xPBD);
    }

    EdgeEdgeCCDState currentState(
        m_vertices[2], m_vertices[3],  // lineMeshA_curr
        m_vertices[6], m_vertices[7]); // lineMeshB_curr
    const double shortestDistanceBetweenLines = currentState.w.norm();
    const double lineDiameter = currentState.thickness();
    EXPECT_NEAR(shortestDistanceBetweenLines, lineDiameter, edgeEdgeCCDTolerance);
}

///
/// \brief Test that two non-colliding line segments do not move.
///
TEST_F(PbdConstraintTest, EdgeEdgeCCDConstraint_TestNoCollision1)
{
    setNumParticles(8);

    m_invMasses.fill(0.0);

    // We place both current and previous vertices in one array
    // In imstk previous vertices are put in a virtual buffer of particles

    // lineMeshA_prev
    m_vertices[0] = Vec3d(0.00, 0.00, -0.01);
    m_vertices[1] = Vec3d(0.00, 0.00, 0.01);

    // lineMeshA_curr
    m_vertices[2]  = Vec3d(0.00, -0.01, -0.01);
    m_invMasses[2] = 1.0;
    m_vertices[3]  = Vec3d(0.00, -0.01, 0.01);
    m_invMasses[3] = 1.0;

    // lineMeshB_prev
    m_vertices[4] = Vec3d(-0.01, 0.01, 0.00);
    m_vertices[5] = Vec3d(0.01, 0.01, 0.00);

    // lineMeshB_curr
    m_vertices[6]  = Vec3d(-0.01, 0.02, 0.00);
    m_invMasses[6] = 1.0;
    m_vertices[7]  = Vec3d(0.01, 0.02, 0.00);
    m_invMasses[7] = 1.0;

    // Values of points for A and B lines before resolving collision.
    Vec3d initA[] = { m_vertices[2], m_vertices[3] };
    Vec3d initB[] = { m_vertices[6], m_vertices[7] };

    PbdEdgeEdgeCCDConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        &m_vertices[0], &m_vertices[1], // prevA
        &m_vertices[4], &m_vertices[5], // prevB
        { 0, 2 }, { 0, 3 },             // currA
        { 0, 6 }, { 0, 7 },             // currB
        0.1, 0.1);

    // 100 iterations is a large number, but the test is simply to verify convergence.
    // In practice, fewer iterations can work.
    for (int i = 0; i < 100; i++)
    {
        solve(0.05, PbdConstraint::SolverType::xPBD);
    }

    // Expect no change in positions (because there was no collision).
    EXPECT_EQ(initA[0], m_vertices[2]);
    EXPECT_EQ(initA[1], m_vertices[3]);
    EXPECT_EQ(initB[0], m_vertices[6]);
    EXPECT_EQ(initB[1], m_vertices[7]);
}