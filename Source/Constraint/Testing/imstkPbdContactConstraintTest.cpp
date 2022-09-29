/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdContactConstraint.h"
#include "imstkPbdConstraintTest.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that the point meets the line and line meets the point
///
TEST_F(MultiBodyPbdConstraintTest, BodyToBodyNormalConstraint_TestConvergence1)
{
    // Setup two bodies
    setNumBodies(2);

    PbdBody& body0 = *m_state.m_bodies[0];
    PbdBody& body1 = *m_state.m_bodies[1];

    // Both rigid bodies (single particle with orientation)
    setNumParticles(body0, 1, true);
    setNumParticles(body1, 1, true);

    // Body A doesn't move
    (*body0.vertices)[0]  = Vec3d(0.0, 0.0, 0.0);
    (*body0.invMasses)[0] = 0.0;
    (*body1.vertices)[0]  = Vec3d(-0.5, -0.75, 0.0);
    (*body1.invMasses)[0] = 1.0;

    // Resolves body B contact point to meet A along contact normal
    // Linear movement along N, or rotation around cross(N, (pt-body center))
    PbdBodyToBodyNormalConstraint constraint;
    m_constraint = &constraint;
    const Vec3d contactPtB = Vec3d(-0.5, -0.25, 0.0);
    constraint.initConstraint(m_state,
        { 0, 0 }, Vec3d(-0.5, -0.5, 0.0), // Body A
        { 1, 0 }, contactPtB,             // Body B
        Vec3d(0.0, 1.0, 0.0),             // N
        0.0);

    const Vec3d initLocalPos = contactPtB - (*body1.vertices)[0];
    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Body B center and orientation should be moved such that contact point
    // moves up to meet contact point on Body A along the contact normal N
    const Vec3d solvedContactPt = (*body1.vertices)[0] + (*body1.orientations)[0]._transformVector(initLocalPos);
    EXPECT_NEAR(-0.5, solvedContactPt[1], IMSTK_DOUBLE_EPS);
}

///
/// \brief Test that the point meets the line and line meets the point
///
TEST_F(MultiBodyPbdConstraintTest, RigidLineToPointConstraint_TestConvergence1)
{
    // Setup two bodies
    setNumBodies(2);

    PbdBody& lineBody = *m_state.m_bodies[0];
    PbdBody& ptBody   = *m_state.m_bodies[1];

    setNumParticles(lineBody, 1, true);
    setNumParticles(ptBody, 1, false);

    (*lineBody.vertices)[0]  = Vec3d(0.0, 0.0, 0.0);
    (*lineBody.invMasses)[0] = 1.0;
    (*ptBody.vertices)[0]    = Vec3d(0.5, 0.0, 0.0);
    (*ptBody.invMasses)[0]   = 0.0; // Pt doesn't move

    PbdRigidLineToPointConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(m_state,
        { 0, 0 }, Vec3d(0.0, 0.5, 0.0), Vec3d(0.0, -0.5, 0.0), // Line
        { 1, 0 },                                              // Pt
        0.0);

    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Line should resolve to pt on the x axes
    EXPECT_NEAR(0.5, (*lineBody.vertices)[0][0], IMSTK_DOUBLE_EPS);
    // Point shouldn't move
    EXPECT_NEAR(0.5, (*ptBody.vertices)[0][0], IMSTK_DOUBLE_EPS);
}