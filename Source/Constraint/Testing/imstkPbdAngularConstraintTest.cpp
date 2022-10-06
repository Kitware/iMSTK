/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdAngularConstraint.h"
#include "imstkPbdConstraintTest.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that the particle with orientation rotates to the other
/// along the hinge, but is free to rotate on one axes
///
TEST_F(PbdConstraintTest, AngularHingeConstraint_TestConvergence1)
{
    setNumParticles(1);

    m_vertices[0]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[0] = Quatd::Identity();
    m_invMasses[0]    = 1.0;

    // Hinge axes restores y of the initial rotational basis to hinge axes
    //const Vec3d initAxes = m_orientations[0].toRotationMatrix().col(1);
    PbdAngularHingeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 0 },             // Particle id
        Vec3d(1.0, 0.0, 0.0), // Hinge axes
        0.0);                 // Compliance

    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check the orientation has returned to x (identity quat)
    const Vec3d newAxes = m_orientations[0].toRotationMatrix().col(1);
    EXPECT_TRUE(Vec3d(1.0, 0.0, 0.0).isApprox(newAxes, IMSTK_DOUBLE_EPS));
}

///
/// \brief Test rotation along the hinge axes doesn't effect the rotation at all
///
TEST_F(PbdConstraintTest, AngularHingeConstraint_TestConvergence2)
{
    setNumParticles(1);

    m_vertices[0]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[0] = Quatd::FromTwoVectors(Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 0.0, 1.0));
    m_invMasses[0]    = 1.0;

    const Quatd initOrientation = m_orientations[0];

    // Hinge axes restores y of the initial rotational basis to hinge axes
    PbdAngularHingeConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 0 },             // Particle id
        Vec3d(0.0, 1.0, 0.0), // Hinge axes
        0.0);                 // Compliance

    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check the orientation hasn't changed from the initial
    EXPECT_TRUE(initOrientation.isApprox(m_orientations[0], IMSTK_DOUBLE_EPS));
}

///
/// \brief Test that the particle with orientation rotates to the other
/// with 0 angular distance
///
TEST_F(PbdConstraintTest, AngularDistanceConstraint_TestConvergenceNoOffset)
{
    setNumParticles(2);

    m_vertices[0]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[0] = Quatd::Identity();
    m_invMasses[0]    = 1.0;

    m_vertices[1]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[1] = Quatd::FromTwoVectors(Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    m_invMasses[1]    = 1.0;

    // Rotates one particle to match the rotation of the other
    PbdAngularDistanceConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraint(
        { 0, 0 },
        { 0, 1 },
        0.0); // Compliance

    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check the orientations now match
    EXPECT_TRUE(m_orientations[0].isApprox(m_orientations[1], 0.00001)) <<
        "Orientation0: " << m_orientations[0] << "\nOrientation1: " << m_orientations[1];
}

///
/// \brief Test that the particle with orientation rotates to the other
/// with a given angular offset
///
TEST_F(PbdConstraintTest, AngularDistanceConstraint_TestConvergenceWithOffset)
{
    setNumParticles(2);

    m_vertices[0]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[0] = Quatd::Identity();
    m_invMasses[0]    = 1.0;
    m_invInterias[0]  = Mat3d::Zero(); // Unrotatable

    m_vertices[1]     = Vec3d(0.0, 0.0, 0.0);
    m_orientations[1] = Quatd::Identity();
    m_invMasses[1]    = 1.0;
    m_invInterias[1]  = Mat3d::Identity();

    const Quatd offset = Quatd::FromTwoVectors(Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));

    // Rotates one particle to match the rotation of the other
    PbdAngularDistanceConstraint constraint;
    m_constraint = &constraint;
    constraint.initConstraintOffset(
        { 0, 0 },
        { 0, 1 },
        offset, // Rotational offset
        0.0);   // Compliance

    for (int i = 0; i < 500; i++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // basis vector x, initially pointing Vec3d(1.0, 0.0, 0.0). Should rotate around z, now pointing up
    const Mat3d rotation = m_orientations[1].toRotationMatrix();
    EXPECT_TRUE(rotation.col(0).isApprox(Vec3d(0.0, 1.0, 0.0))) <<
        "\nExpected Basis x: 0.0, 1.0, 0.0" <<
        "\n  Actual Basis x: " << rotation.col(0).transpose();
    EXPECT_TRUE(rotation.col(1).isApprox(Vec3d(-1.0, 0.0, 0.0))) <<
        "\nExpected Basis y: -1.0, 0.0, 0.0" <<
        "\n  Actual Basis y: " << rotation.col(1).transpose();
    // z should remain unchanged
    EXPECT_TRUE(rotation.col(2).isApprox(Vec3d(0.0, 0.0, 1.0))) <<
        "\nExpected Basis z: 0.0, 0.0, 1.0" <<
        "\n  Actual Basis z: " << rotation.col(2).transpose();
}