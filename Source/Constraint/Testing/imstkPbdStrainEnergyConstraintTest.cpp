/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdConstraintTest.h"
#include "imstkPbdStrainEnergyTetConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that tet will right itself if inverted using the StVK model
///
TEST_F(PbdConstraintTest, StrainEnergyTetConstraint_TestTetInversionStVK)
{
    setNumParticles(4);

    m_vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m_invMasses[0] = 400.0;
    m_invMasses[1] = 400.0;
    m_invMasses[2] = 400.0;
    m_invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    // PbdStrainEnergyConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto secConfig = std::make_shared<PbdStrainEnergyConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    // Set up constraint
    PbdStrainEnergyTetConstraint constraint(PbdStrainEnergyConstraint::MaterialType::StVK);
    m_constraint = &constraint;
    constraint.initConstraint(
        m_vertices[0], m_vertices[1], m_vertices[2], m_vertices[3],
        { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, secConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    m_vertices[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}

///
/// \brief Test that tet will right itself if inverted using the NeoHookean model
///
TEST_F(PbdConstraintTest, StrainEnergyTetConstraint_TestTetInversionNeoHookean)
{
    setNumParticles(4);

    m_vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m_invMasses[0] = 400.0;
    m_invMasses[1] = 400.0;
    m_invMasses[2] = 400.0;
    m_invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    // PbdStrainEnergyConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto secConfig = std::make_shared<PbdStrainEnergyConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    // Set up constraint
    PbdStrainEnergyTetConstraint constraint(PbdStrainEnergyConstraint::MaterialType::NeoHookean);
    m_constraint = &constraint;
    constraint.initConstraint(
        m_vertices[0], m_vertices[1], m_vertices[2], m_vertices[3],
        { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, secConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    m_vertices[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}

///
/// \brief Test that tet will right itself if inverted using the Corotational model
///
TEST_F(PbdConstraintTest, StrainEnergyTetConstraint_TestTetInversionCorotational)
{
    setNumParticles(4);

    m_vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m_invMasses[0] = 400.0;
    m_invMasses[1] = 400.0;
    m_invMasses[2] = 400.0;
    m_invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    // PbdStrainEnergyConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto secConfig = std::make_shared<PbdStrainEnergyConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    // Set up constraint
    PbdStrainEnergyTetConstraint constraint(PbdStrainEnergyConstraint::MaterialType::Corotation);
    m_constraint = &constraint;
    constraint.initConstraint(
        m_vertices[0], m_vertices[1], m_vertices[2], m_vertices[3],
        { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, secConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    m_vertices[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        solve(0.01, PbdConstraint::SolverType::xPBD);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = m_vertices[0] - m_vertices[3];
    m.col(1) = m_vertices[1] - m_vertices[3];
    m.col(2) = m_vertices[2] - m_vertices[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}