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

#include "gtest/gtest.h"

#include "imstkPbdConstraint.h"
#include "imstkPbdFemTetConstraint.h"

using namespace imstk;

///
/// \brief Test that tet will right itself if inverted using the StVK model
///
TEST(imstkPbdFemConstraintTest, TestTetInversionStVK)
{
    // Set up constraint
    PbdFemTetConstraint constraint(PbdFemConstraint::MaterialType::StVK);

    DataArray<double> invMasses(4);
    invMasses[0] = 400.0;
    invMasses[1] = 400.0;
    invMasses[2] = 400.0;
    invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;

    // Setup the Geometry
    auto                     vertices = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>& verts    = *vertices;

    *vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    // PbdFemConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto femConfig = std::make_shared<PbdFemConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    constraint.initConstraint(*vertices, 0, 1, 2, 3, femConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    verts[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        constraint.projectConstraint(invMasses, 0.01, PbdConstraint::SolverType::xPBD, *vertices);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}

///
/// \brief Test that tet will right itself if inverted using the NeoHookean model
///
TEST(imstkPbdFemConstraintTest, TestTetInversionNeoHookean)
{
    // Set up constraint
    PbdFemTetConstraint constraint(PbdFemConstraint::MaterialType::NeoHookean);

    DataArray<double> invMasses(4);
    invMasses[0] = 400.0;
    invMasses[1] = 400.0;
    invMasses[2] = 400.0;
    invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;

    // Setup the Geometry
    auto                     vertices = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>& verts    = *vertices;

    *vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    // PbdFemConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto femConfig = std::make_shared<PbdFemConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    constraint.initConstraint(*vertices, 0, 1, 2, 3, femConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    verts[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        constraint.projectConstraint(invMasses, 0.01, PbdConstraint::SolverType::xPBD, *vertices);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}

///
/// \brief Test that tet will right itself if inverted using the Corotational model
///
TEST(imstkPbdFemConstraintTest, TestTetInversionCorotational)
{
    // Set up constraint
    PbdFemTetConstraint constraint(PbdFemConstraint::MaterialType::Corotation);

    DataArray<double> invMasses(4);
    invMasses[0] = 400.0;
    invMasses[1] = 400.0;
    invMasses[2] = 400.0;
    invMasses[3] = 400.0;

    // Ref position matrix
    Mat3d m;

    // Setup the Geometry
    auto                     vertices = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>& verts    = *vertices;

    *vertices = {
        { 0.5, 0.0, -1.0 / 3.0 },
        { -0.5, 0.0, -1.0 / 3.0 },
        { 0.0, 0.0, 2.0 / 3.0 },
        { 0.0, 1.0, 0.0 },
    };

    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    VecDataArray<int, 4> indices(1);
    indices[0] = Vec4i(0, 1, 2, 3);

    // PbdFemConstraintConfig(mu, lambda, youngModulus, poissonRatio)
    auto femConfig = std::make_shared<PbdFemConstraintConfig>(344.82, 3103.44, 1000.0, 0.45);

    constraint.initConstraint(*vertices, 0, 1, 2, 3, femConfig);

    Mat3d refPositionInverse = m.inverse();

    // Invert the tet
    verts[3] += Vec3d(0.1, -2.6, -0.1);

    for (int step = 0; step < 600; step++)
    {
        constraint.projectConstraint(invMasses, 0.01, PbdConstraint::SolverType::xPBD, *vertices);
    }

    // Check sign of determinant of deformation gradient
    m.col(0) = verts[0] - verts[3];
    m.col(1) = verts[1] - verts[3];
    m.col(2) = verts[2] - verts[3];

    Mat3d F    = m * refPositionInverse;
    auto  detF = F.determinant();

    EXPECT_TRUE(detF > 0);
}