/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkPbdBendConstraint.h"

using namespace imstk;

///
/// \brief Test that two connecting line segments unfold
///
TEST(imstkPbdBendConstraintTest, TestConvergence1)
{
    PbdBendConstraint constraint;

    // Straight line upon initialization
    VecDataArray<double, 3> vertices(3);
    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    vertices[1] = Vec3d(0.5, 0.0, 0.0);
    vertices[2] = Vec3d(1.0, 0.0, 0.0);
    DataArray<double> invMasses(3);
    invMasses[0] = 1.0;
    invMasses[1] = 0.0; // Center doesn't move
    invMasses[2] = 1.0;

    constraint.initConstraint(vertices, 0, 1, 2, 1e20);

    // Modify it so the line segments look like \/
    vertices[0][1] = 0.1;
    vertices[2][1] = 0.1;
    for (int i = 0; i < 500; i++)
    {
        constraint.projectConstraint(invMasses, 0.01, PbdConstraint::SolverType::xPBD, vertices);
    }

    // Should resolve back to a flat line
    EXPECT_NEAR(vertices[0][1], 0.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(vertices[2][1], 0.0, IMSTK_DOUBLE_EPS);
}