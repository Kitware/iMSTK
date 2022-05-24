/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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