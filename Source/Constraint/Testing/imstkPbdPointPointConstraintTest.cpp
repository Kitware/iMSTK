/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraintTest.h"
#include "imstkPbdPointPointConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that two points meet
///
TEST_F(PbdConstraintTest, PointPointConstraint_TestConvergence1)
{
    setNumParticles(2);

    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(0.0, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.0, -1.0, 0.0);

    PbdPointPointConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 0 }, { 0, 1 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    ASSERT_EQ(m_vertices[0][1], m_vertices[1][1]);
}