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

#include "imstkPbdPointPointConstraint.h"

using namespace imstk;

///
/// \brief TODO
///
class imstkPbdPointPointConstraintTest : public ::testing::Test
{
protected:
    PbdPointPointConstraint m_constraint;
};

///
/// \brief Test that two points meet
///
TEST_F(imstkPbdPointPointConstraintTest, TestConvergence1)
{
    Vec3d a = Vec3d(0.0, 0.0, 0.0);
    Vec3d b = Vec3d(0.0, -1.0, 0.0);

    m_constraint.initConstraint(
        { &a, 1.0, nullptr },
        { &b, 1.0, nullptr },
        1.0, 1.0);
    for (int i = 0; i < 3; i++)
    {
        m_constraint.solvePosition();
    }

    ASSERT_EQ(a[1], b[1]);
}

///
/// \brief TODO
///
int
imstkPbdPointPointConstraintTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
