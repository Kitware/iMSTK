/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkPbdPointPointConstraint.h"

using namespace imstk;

///
/// \brief Test that two points meet
///
TEST(imstkPbdPointPointConstraintTest, TestConvergence1)
{
    PbdPointPointConstraint constraint;

    Vec3d a = Vec3d(0.0, 0.0, 0.0);
    Vec3d b = Vec3d(0.0, -1.0, 0.0);

    constraint.initConstraint(
        { &a, 1.0, nullptr },
        { &b, 1.0, nullptr },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        constraint.solvePosition();
    }

    ASSERT_EQ(a[1], b[1]);
}