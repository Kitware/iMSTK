/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraintTest.h"
#include "imstkPbdPointTriangleConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that a point below a triangle, and the triangle, meet on y axes
///
TEST_F(PbdConstraintTest, PointTriangleConstraint_TestConvergence1)
{
    setNumParticles(4);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(0.5, 0.0, -0.5);
    m_vertices[1] = Vec3d(-0.5, 0.0, -0.5);
    m_vertices[2] = Vec3d(0.0, 0.0, 0.5);

    const Vec3d& a = m_vertices[0];
    const Vec3d& b = m_vertices[1];
    const Vec3d& c = m_vertices[2];

    m_vertices[3]     = (a + b + c) / 3.0;
    m_vertices[3][1] -= 1.0;

    const Vec3d& x = m_vertices[3];

    PbdPointTriangleConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 3 },
        { 0, 0 }, { 0, 1 }, { 0, 2 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_NEAR(x[1], a[1], 0.00000001);
    EXPECT_NEAR(x[1], b[1], 0.00000001);
    EXPECT_NEAR(x[1], c[1], 0.00000001);
}

///
/// \brief Test that a point above a triangle, and the triangle, meet on y axes
///
TEST_F(PbdConstraintTest, PointTriangleConstraint_TestConvergence2)
{
    setNumParticles(4);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(0.5, 0.0, -0.5);
    m_vertices[1] = Vec3d(-0.5, 0.0, -0.5);
    m_vertices[2] = Vec3d(0.0, 0.0, 0.5);

    const Vec3d& a = m_vertices[0];
    const Vec3d& b = m_vertices[1];
    const Vec3d& c = m_vertices[2];

    m_vertices[3]     = (a + b + c) / 3.0;
    m_vertices[3][1] += 1.0;

    const Vec3d& x = m_vertices[3];

    PbdPointTriangleConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 3 },
        { 0, 0 }, { 0, 1 }, { 0, 2 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_NEAR(x[1], a[1], 0.00000001);
    EXPECT_NEAR(x[1], b[1], 0.00000001);
    EXPECT_NEAR(x[1], c[1], 0.00000001);
}

///
/// \brief Test that a point not within the triangle does not move at all
///
TEST_F(PbdConstraintTest, PointTriangleConstraint_TestNoConvergence1)
{
    setNumParticles(4);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(0.5, 0.0, -0.5);
    Vec3d&      a     = m_vertices[0];
    const Vec3d aInit = a;
    m_vertices[1] = Vec3d(-0.5, 0.0, -0.5);
    Vec3d&      b     = m_vertices[1];
    const Vec3d bInit = b;
    m_vertices[2] = Vec3d(0.0, 0.0, 0.5);
    Vec3d&      c     = m_vertices[2];
    const Vec3d cInit = c;

    // Test all 3 sides of the triangle (u,v,w)
    Vec3d testPts[3] =
    {
        Vec3d(3.0, -1.0, 0.0),
        Vec3d(-3.0, -1.0, 0.0),
        Vec3d(0.0, -1.0, -3.0)
    };
    const Vec3d initTestPts[3] = { testPts[0], testPts[1], testPts[2] };

    for (int i = 0; i < 3; i++)
    {
        // Reset to initial positions
        a = aInit;
        b = bInit;
        c = cInit;

        m_vertices[3] = testPts[i];

        PbdPointTriangleConstraint constraint;
        m_constraint = &constraint;
        constraint.initConstraint(
            { 0, 3 },
            { 0, 0 }, { 0, 1 }, { 0, 2 },
            1.0, 1.0);
        for (int j = 0; j < 3; j++)
        {
            solve(0.01, PbdConstraint::SolverType::PBD);
        }

        // Test they haven't moved
        EXPECT_EQ(initTestPts[i][0], testPts[i][0]) << "i: " << i;
        EXPECT_EQ(initTestPts[i][1], testPts[i][1]) << "i: " << i;
        EXPECT_EQ(initTestPts[i][2], testPts[i][2]) << "i: " << i;

        EXPECT_EQ(aInit[0], a[0]);
        EXPECT_EQ(aInit[1], a[1]);
        EXPECT_EQ(aInit[2], a[2]);

        EXPECT_EQ(bInit[0], b[0]);
        EXPECT_EQ(bInit[1], b[1]);
        EXPECT_EQ(bInit[2], b[2]);

        EXPECT_EQ(cInit[0], c[0]);
        EXPECT_EQ(cInit[1], c[1]);
        EXPECT_EQ(cInit[2], c[2]);
    }
}