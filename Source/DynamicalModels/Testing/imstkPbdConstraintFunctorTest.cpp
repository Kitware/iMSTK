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

#include "imstkLineMesh.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkVecDataArray.h"

using namespace imstk;

///
/// \brief Test that the correct constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestGeneration1)
{
    // Create mesh for generation
    auto lineMesh = std::make_shared<LineMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(3);
    (*vertices)[0] = Vec3d(-0.5, 0.0, 0.0);
    (*vertices)[1] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.5, 0.0, 0.0);
    auto indices = std::make_shared<VecDataArray<int, 2>>(2);
    (*indices)[0] = Vec2i(0, 1);
    (*indices)[1] = Vec2i(1, 2);
    lineMesh->initialize(vertices, indices);

    // Create functor
    PbdBendConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1e20);
    constraintFunctor.setStride(1);
    constraintFunctor.setGeometry(lineMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    std::shared_ptr<PbdBendConstraint> constraint =
        std::dynamic_pointer_cast<PbdBendConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1e20);
    EXPECT_EQ(constraint->getVertexIds().size(), 3);
    EXPECT_EQ(constraint->getVertexIds()[0], 0);
    EXPECT_EQ(constraint->getVertexIds()[1], 1);
    EXPECT_EQ(constraint->getVertexIds()[2], 2);
}

///
/// \brief Test that the correct constraint was generated with differing stride
///
TEST(imstkPbdConstraintFunctorTest, TestGeneration2)
{
    // Create mesh for generation
    auto lineMesh = std::make_shared<LineMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(5);
    (*vertices)[0] = Vec3d(-1.0, 0.0, 0.0);
    (*vertices)[1] = Vec3d(-0.5, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[3] = Vec3d(0.5, 0.0, 0.0);
    (*vertices)[4] = Vec3d(1.0, 0.0, 0.0);
    auto indices = std::make_shared<VecDataArray<int, 2>>(4);
    (*indices)[0] = Vec2i(0, 1);
    (*indices)[1] = Vec2i(1, 2);
    (*indices)[2] = Vec2i(2, 3);
    (*indices)[3] = Vec2i(3, 4);
    lineMesh->initialize(vertices, indices);

    // Create functor
    PbdBendConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1e20);
    constraintFunctor.setStride(2);
    constraintFunctor.setGeometry(lineMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    std::shared_ptr<PbdBendConstraint> constraint =
        std::dynamic_pointer_cast<PbdBendConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1e20);
    EXPECT_EQ(constraint->getVertexIds().size(), 3);
    EXPECT_EQ(constraint->getVertexIds()[0], 0);
    EXPECT_EQ(constraint->getVertexIds()[1], 2);
    EXPECT_EQ(constraint->getVertexIds()[2], 4);
}