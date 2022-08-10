/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraintTest.h"
#include "imstkPbdPointEdgeConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that a point and edge meet on touching
/// point below edge
///
TEST_F(PbdConstraintTest, PointEdgeConstraint_TestConvergence1)
{
    setNumParticles(3);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(-0.5, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.5, 0.0, 0.0);
    m_vertices[2] = Vec3d(0.0, -1.0, 0.0);

    const Vec3d& a = m_vertices[0];
    const Vec3d& b = m_vertices[1];
    const Vec3d& x = m_vertices[2];

    PbdPointEdgeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 2 }, { 0, 0 }, { 0, 1 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_NEAR(x[1], a[1], IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(x[1], b[1], IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(a[1], b[1], IMSTK_DOUBLE_EPS);
}

///
/// \brief Test that a point and edge meet on touching
/// point above edge
///
TEST_F(PbdConstraintTest, PointEdgeConstraint_TestConvergence2)
{
    setNumParticles(3);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(-0.5, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.5, 0.0, 0.0);
    m_vertices[2] = Vec3d(0.0, 1.0, 0.0);

    const Vec3d& a = m_vertices[0];
    const Vec3d& b = m_vertices[1];
    const Vec3d& x = m_vertices[2];

    PbdPointEdgeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 2 }, { 0, 0 }, { 0, 1 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_NEAR(x[1], a[1], IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(x[1], b[1], IMSTK_DOUBLE_EPS);
    EXPECT_NEAR(a[1], b[1], IMSTK_DOUBLE_EPS);
}

///
/// \brief Test that a point not within bounds of edge does not move (left)
///
TEST_F(PbdConstraintTest, PointEdgeConstraint_TestNoConvergence1)
{
    setNumParticles(3);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(-0.5, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.5, 0.0, 0.0);
    m_vertices[2] = Vec3d(-1.0, -1.0, 0.0);

    const Vec3d& a     = m_vertices[0];
    const Vec3d  aInit = a;
    const Vec3d& b     = m_vertices[1];
    const Vec3d  bInit = b;
    const Vec3d& x     = m_vertices[2];
    const Vec3d  xInit = x;

    PbdPointEdgeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 2 }, { 0, 0 }, { 0, 1 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_EQ(xInit[0], x[0]);
    EXPECT_EQ(xInit[1], x[1]);
    EXPECT_EQ(xInit[2], x[2]);

    EXPECT_EQ(aInit[0], a[0]);
    EXPECT_EQ(aInit[1], a[1]);
    EXPECT_EQ(aInit[2], a[2]);

    EXPECT_EQ(bInit[0], b[0]);
    EXPECT_EQ(bInit[1], b[1]);
    EXPECT_EQ(bInit[2], b[2]);
}

///
/// \brief Test that a point not within bounds of edge does not move (right)
///
TEST_F(PbdConstraintTest, PointEdgeConstraint_TestNoConvergence2)
{
    setNumParticles(3);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(-0.5, 0.0, 0.0);
    m_vertices[1] = Vec3d(0.5, 0.0, 0.0);
    m_vertices[2] = Vec3d(1.0, -1.0, 0.0);

    const Vec3d& a     = m_vertices[0];
    const Vec3d  aInit = a;
    const Vec3d& b     = m_vertices[1];
    const Vec3d  bInit = b;
    const Vec3d& x     = m_vertices[2];
    const Vec3d  xInit = x;

    PbdPointEdgeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 2 }, { 0, 0 }, { 0, 1 },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        solve(0.01, PbdConstraint::SolverType::PBD);
    }

    EXPECT_EQ(xInit[0], x[0]);
    EXPECT_EQ(xInit[1], x[1]);
    EXPECT_EQ(xInit[2], x[2]);

    EXPECT_EQ(aInit[0], a[0]);
    EXPECT_EQ(aInit[1], a[1]);
    EXPECT_EQ(aInit[2], a[2]);

    EXPECT_EQ(bInit[0], b[0]);
    EXPECT_EQ(bInit[1], b[1]);
    EXPECT_EQ(bInit[2], b[2]);
}