/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPbdConstraintTest.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test that the point from an element moves to a
/// point on an another element.
///
/// This tests the following cases:
/// point vs { point, edge, triangle, tet }
/// edge vs { point, edge, triangle, tet }
/// triangle vs { point, edge, triangle tet }
/// tet vs { point, edge, triangle, tet }
///
TEST_F(PbdConstraintTest, BaryPointToPointConstraint_TestConvergence)
{
    setNumParticles(8);
    m_invMasses.fill(1.0);

    m_vertices[0] = Vec3d(1.0, 0.0, -1.0 / std::sqrt(2.0));
    m_vertices[1] = Vec3d(-1.0, 0.0, -1.0 / std::sqrt(2.0));
    m_vertices[2] = Vec3d(0.0, 1.0, 1.0 / std::sqrt(2.0));
    m_vertices[3] = Vec3d(0.0, -1.0, 1.0 / std::sqrt(2.0));
    std::vector<std::vector<double>> weightsA =
    {
        { 1.0 },
        { 0.2, 0.8 },
        { 0.2, 0.6, 0.2 },
        { 0.1, 0.1, 0.2, 0.6 }
    };

    m_vertices[4] = Vec3d(-0.5, -1.0, 0.0);
    m_vertices[5] = Vec3d(0.5, -1.0, 0.0);
    m_vertices[6] = Vec3d(0.0, 1.0, 0.0);
    m_vertices[7] = Vec3d(0.0, 1.0, 1.0);
    std::vector<std::vector<double>> weightsB =
    {
        { 1.0 },
        { 0.5, 0.5 },
        { 0.3, 0.6, 0.1 },
        { 0.2, 0.3, 0.2, 0.3 }
    };

    std::array<std::string, 4> elementNameStr =
    {
        "Point",
        "Edge",
        "Triangle",
        "Tetra"
    };

    for (size_t i = 0; i < 4; i++)
    {
        std::vector<PbdParticleId> pointsA(i + 1);
        for (size_t k = 0; k < pointsA.size(); k++)
        {
            pointsA[k] = { 0, static_cast<int>(k) };
        }

        for (size_t j = 0; j < 4; j++)
        {
            std::vector<PbdParticleId> pointsB(j + 1);
            for (size_t k = 0; k < pointsB.size(); k++)
            {
                pointsB[k] = { 0, static_cast<int>(k + 4) };
            }

            PbdBaryPointToPointConstraint constraint;
            m_constraint = &constraint;
            constraint.initConstraint(
                pointsA, weightsA[i],
                pointsB, weightsB[j],
                1.0, 1.0);
            for (int k = 0; k < 3; k++)
            {
                solve(0.01, PbdConstraint::SolverType::PBD);
            }

            // Compute the resulting interpolated points
            Vec3d resultA = Vec3d::Zero();
            for (size_t k = 0; k < pointsA.size(); k++)
            {
                resultA += m_vertices[pointsA[k].second] * weightsA[i][k];
            }
            Vec3d resultB = Vec3d::Zero();
            for (size_t k = 0; k < pointsB.size(); k++)
            {
                resultB += m_vertices[pointsB[k].second] * weightsB[j][k];
            }

            // Assert that the barycentric point on the line now is equivalent to a
            ASSERT_TRUE(resultA.isApprox(resultB)) <<
                "Type: " << elementNameStr[i] << "To" << elementNameStr[j] << std::endl <<
                "resultA: " << resultA.transpose() << std::endl <<
                "resultB: " << resultB.transpose();
        }
    }
}