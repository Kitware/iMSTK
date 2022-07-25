/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkEdgeEdgeCCDState.h"
#include "imstkLineMeshToLineMeshCCD.h"
#include "imstkPbdEdgeEdgeCCDConstraint.h"

using namespace imstk;
static const double edgeEdgeCCDTolerance = 1e-6;

///
/// \brief Test that two colliding line segments resolve.
///
TEST(imstkPbdEdgeEdgeCCDConstraintTest, TestCollision1)
{
    Vec3d lineMeshA_prev[] = { Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01) };
    Vec3d lineMeshA_curr[] = { Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01) };
    Vec3d lineMeshB_prev[] = { Vec3d(-0.01, 0.01, 0.00), Vec3d(0.01, 0.01, 0.00) };
    Vec3d lineMeshB_curr[] = { Vec3d(-0.01, -0.01, 0.00), Vec3d(0.01, -0.01, 0.00) };

    PbdEdgeEdgeCCDConstraint constraint;

    Vec3d zeroVelocity = Vec3d::Zero();

    constraint.initConstraint(
        { &lineMeshA_prev[0], 0.0, &zeroVelocity }, { &lineMeshA_prev[1], 0.0, &zeroVelocity },
        { &lineMeshB_prev[0], 0.0, &zeroVelocity }, { &lineMeshB_prev[1], 0.0, &zeroVelocity },
        { &lineMeshA_curr[0], 0.0, &zeroVelocity }, { &lineMeshA_curr[1], 0.0, &zeroVelocity },
        { &lineMeshB_curr[0], 1.0, &zeroVelocity }, { &lineMeshB_curr[1], 1.0, &zeroVelocity },

        0.1, 0.1
        );

    // 100 iterations is a large number, but the test is simply to verify convergence.
    // In practice, fewer iterations can work.
    for (int i = 0; i < 100; i++)
    {
        constraint.solvePosition();
    }

    EdgeEdgeCCDState currentState(lineMeshA_curr[0], lineMeshA_curr[1], lineMeshB_curr[0], lineMeshB_curr[1]);
    const double     shortestDistanceBetweenLines = currentState.w.norm();
    const double     lineDiameter = currentState.thickness();
    EXPECT_NEAR(shortestDistanceBetweenLines, lineDiameter, edgeEdgeCCDTolerance);
}

///
/// \brief Test that two non-colliding line segments do not move.
///
TEST(imstkPbdEdgeEdgeCCDConstraintTest, TestNoCollision1)
{
    Vec3d lineMeshA_prev[] = { Vec3d(0.00, 0.00, -0.01), Vec3d(0.00, 0.00, 0.01) };
    Vec3d lineMeshA_curr[] = { Vec3d(0.00, -0.01, -0.01), Vec3d(0.00, -0.01, 0.01) };
    Vec3d lineMeshB_prev[] = { Vec3d(-0.01, 0.01, 0.00), Vec3d(0.01, 0.01, 0.00) };
    Vec3d lineMeshB_curr[] = { Vec3d(-0.01, 0.02, 0.00), Vec3d(0.01, 0.02, 0.00) };

    // Values of points for A and B lines before resolving collision.
    Vec3d initA[] = { lineMeshA_curr[0], lineMeshA_curr[1] };
    Vec3d initB[] = { lineMeshB_curr[0], lineMeshB_curr[1] };

    PbdEdgeEdgeCCDConstraint constraint;

    Vec3d zeroVelocity = Vec3d::Zero();

    constraint.initConstraint(
        { &lineMeshA_prev[0], 0.0, &zeroVelocity }, { &lineMeshA_prev[1], 0.0, &zeroVelocity },
        { &lineMeshB_prev[0], 0.0, &zeroVelocity }, { &lineMeshB_prev[1], 0.0, &zeroVelocity },
        { &lineMeshA_curr[0], 1.0, &zeroVelocity }, { &lineMeshA_curr[1], 1.0, &zeroVelocity },
        { &lineMeshB_curr[0], 1.0, &zeroVelocity }, { &lineMeshB_curr[1], 1.0, &zeroVelocity },

        0.1, 0.1
        );

    // 100 iterations is a large number, but the test is simply to verify convergence.
    // In practice, fewer iterations can work.
    for (int i = 0; i < 100; i++)
    {
        constraint.solvePosition();
    }

    // Expect no change in positions (because there was no collision).
    EXPECT_EQ(initA[0], lineMeshA_curr[0]);
    EXPECT_EQ(initA[1], lineMeshA_curr[1]);
    EXPECT_EQ(initB[0], lineMeshB_curr[0]);
    EXPECT_EQ(initB[1], lineMeshB_curr[1]);
}