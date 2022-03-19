/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkPbdBaryPointToPointConstraint.h"

#include <gtest/gtest.h>

using namespace imstk;

static void
solveConstraint(
    std::vector<Vec3d>& inPts_a, const std::vector<double>& weights_a,
    std::vector<Vec3d>& inPts_b, const std::vector<double>& weights_b,
    Vec3d& resultA, Vec3d& resultB)
{
    const size_t numPtsA = inPts_a.size();
    const size_t numPtsB = inPts_b.size();

    std::vector<VertexMassPair> pts_a(numPtsA);
    for (size_t i = 0; i < numPtsA; i++)
    {
        pts_a[i] = { &inPts_a[i], 1.0, nullptr };
    }
    std::vector<VertexMassPair> pts_b(numPtsB);
    for (size_t i = 0; i < numPtsB; i++)
    {
        pts_b[i] = { &inPts_b[i], 1.0, nullptr };
    }

    PbdBaryPointToPointConstraint constraint;
    constraint.initConstraint(
        pts_a, weights_a,
        pts_b, weights_b,
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        constraint.solvePosition();
    }

    // Compute the resulting interpolated points
    resultA = Vec3d::Zero();
    for (size_t i = 0; i < numPtsA; i++)
    {
        resultA += *pts_a[i].vertex * weights_a[i];
    }
    resultB = Vec3d::Zero();
    for (size_t i = 0; i < numPtsB; i++)
    {
        resultB += *pts_b[i].vertex * weights_b[i];
    }
}

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
TEST(PbdBaryPointToPointConstraintTest, TestConvergence)
{
    std::array<Vec3d, 4> pts_a =
    {
        Vec3d(1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(-1.0, 0.0, -1.0 / std::sqrt(2.0)),
        Vec3d(0.0, 1.0, 1.0 / std::sqrt(2.0)),
        Vec3d(0.0, -1.0, 1.0 / std::sqrt(2.0))
    };
    std::vector<std::vector<double>> weightsA =
    {
        { 1.0 },
        { 0.2, 0.8 },
        { 0.2, 0.6, 0.2 },
        { 0.1, 0.1, 0.2, 0.6 }
    };
    std::array<Vec3d, 4> pts_b =
    {
        Vec3d(-0.5, -1.0, 0.0),
        Vec3d(0.5, -1.0, 0.0),
        Vec3d(0.0, 1.0, 0.0),
        Vec3d(0.0, 1.0, 1.0)
    };
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
        std::vector<Vec3d> pointsA(i + 1);
        for (size_t k = 0; k < pointsA.size(); k++)
        {
            pointsA[k] = pts_a[k];
        }

        for (size_t j = 0; j < 4; j++)
        {
            std::vector<Vec3d> pointsB(j + 1);
            for (size_t k = 0; k < pointsB.size(); k++)
            {
                pointsB[k] = pts_b[k];
            }

            Vec3d resultA, resultB;
            solveConstraint(
                pointsA, weightsA[i],
                pointsB, weightsB[j],
                resultA, resultB);

            // Assert that the barycentric point on the line now is equivalent to a
            ASSERT_TRUE(resultA.isApprox(resultB)) <<
                "Type: " << elementNameStr[i] << "To" << elementNameStr[j] << std::endl <<
                "resultA: " << resultA.transpose() << std::endl <<
                "resultB: " << resultB.transpose();
        }
    }
}