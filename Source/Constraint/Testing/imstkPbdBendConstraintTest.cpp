/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdBendConstraint.h"
#include "imstkPbdConstraintTest.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that two connecting line segments unfold
///
TEST_F(PbdConstraintTest, BendConstraint_TestConvergence1)
{
    setNumParticles(3);

    // Straight line upon initialization
    m_vertices[0] = Vec3d(0.0, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.5, 0.0, 0.0);
    m_vertices[2] = Vec3d(1.0, 0.0, 0.0);

    m_invMasses[0] = 1.0;
    m_invMasses[1] = 0.0; // Center doesn't move
    m_invMasses[2] = 1.0;

    PbdBendConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        m_vertices[0], m_vertices[1], m_vertices[2],
        { 0, 0 }, { 0, 1 }, { 0, 2 }, 1e20);

    // Modify it so the line segments look like \/
    m_vertices[0][1] = 0.1;
    m_vertices[2][1] = 0.1;
    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Should resolve back to a flat line
    EXPECT_NEAR(m_vertices[0][1], 0.0, IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(m_vertices[2][1], 0.0, IMSTK_DOUBLE_EPS);
}