/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkLineMesh.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkVecDataArray.h"

using namespace imstk;

///
/// \brief Test that the bending correct constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestBendingConstraintStride1Generation)
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
    auto constraint = std::dynamic_pointer_cast<PbdBendConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1e20);
    EXPECT_EQ(constraint->getParticles().size(), 3);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 1);
    EXPECT_EQ(constraint->getParticles()[2].second, 2);
}

///
/// \brief Test that the correct bending constraint was generated with differing stride
///
TEST(imstkPbdConstraintFunctorTest, TestBendingConstraintStride2Generation)
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
    auto constraint = std::dynamic_pointer_cast<PbdBendConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1e20);
    EXPECT_EQ(constraint->getParticles().size(), 3);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 2);
    EXPECT_EQ(constraint->getParticles()[2].second, 4);
}

///
/// \brief Test that the correct distance constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestDistanceConstraintGeneration)
{
    // Create mesh for generation
    auto lineMesh = std::make_shared<LineMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(2);
    (*vertices)[0] = Vec3d(-0.5, 0.0, 0.0);
    (*vertices)[1] = Vec3d(0.0, 0.0, 0.0);
    auto indices = std::make_shared<VecDataArray<int, 2>>(1);
    (*indices)[0] = Vec2i(0, 1);
    lineMesh->initialize(vertices, indices);

    // Create functor
    PbdDistanceConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1.0e3);
    constraintFunctor.setGeometry(lineMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    auto constraint = std::dynamic_pointer_cast<PbdDistanceConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1.0e3);
    EXPECT_EQ(constraint->getParticles().size(), 2);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 1);
}

///
/// \brief Test that the correct pbd strain energy tetrahedral constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestStrainEnergyTetConstraintGeneration)
{
    auto tetMesh  = std::make_shared<TetrahedralMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(4);
    (*vertices)[0] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[1] = Vec3d(1.0, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.0, 1.0, 0.0);
    (*vertices)[3] = Vec3d(0.0, 0.0, 1.0);
    auto indices = std::make_shared<VecDataArray<int, 4>>(1);
    (*indices)[0] = Vec4i(0, 1, 2, 3);
    tetMesh->initialize(vertices, indices);

    // Create functor
    PbdStrainEnergyTetConstraintFunctor constraintFunctor;
    constraintFunctor.setMaterialType(PbdStrainEnergyTetConstraint::MaterialType::Corotation);
    auto seConfig = std::make_shared<PbdStrainEnergyConstraintConfig>(0.0, 0.0, 1000.0, 0.2);
    constraintFunctor.setSecConfig(seConfig);
    constraintFunctor.setGeometry(tetMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    auto constraint = std::dynamic_pointer_cast<PbdStrainEnergyTetConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->m_material, PbdStrainEnergyTetConstraint::MaterialType::Corotation);
    EXPECT_EQ(constraint->m_config->m_mu, 0.0);
    EXPECT_EQ(constraint->m_config->m_lambda, 0.0);
    EXPECT_EQ(constraint->m_config->m_YoungModulus, 1000.0);
    EXPECT_EQ(constraint->m_config->m_PoissonRatio, 0.2);
    EXPECT_EQ(constraint->getParticles().size(), 4);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 1);
    EXPECT_EQ(constraint->getParticles()[2].second, 2);
    EXPECT_EQ(constraint->getParticles()[3].second, 3);
}

///
/// \brief Test that the correct pbd volume constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestVolumeConstraintGeneration)
{
    auto tetMesh  = std::make_shared<TetrahedralMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(4);
    (*vertices)[0] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[1] = Vec3d(1.0, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.0, 1.0, 0.0);
    (*vertices)[3] = Vec3d(0.0, 0.0, 1.0);
    auto indices = std::make_shared<VecDataArray<int, 4>>(1);
    (*indices)[0] = Vec4i(0, 1, 2, 3);
    tetMesh->initialize(vertices, indices);

    // Create functor
    PbdVolumeConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1.0e4);
    constraintFunctor.setGeometry(tetMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    auto constraint = std::dynamic_pointer_cast<PbdVolumeConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1.0e4);
    EXPECT_EQ(constraint->getParticles().size(), 4);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 1);
    EXPECT_EQ(constraint->getParticles()[2].second, 2);
    EXPECT_EQ(constraint->getParticles()[3].second, 3);
}

///
/// \brief Test that the correct pbd area constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestAreaConstraintGeneration)
{
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(4);
    (*vertices)[0] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[1] = Vec3d(1.0, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.0, 1.0, 0.0);
    (*vertices)[3] = Vec3d(0.0, 0.0, 1.0);
    auto indices = std::make_shared<VecDataArray<int, 3>>(1);
    (*indices)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(vertices, indices);

    // Create functor
    PbdAreaConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1.0e4);
    constraintFunctor.setGeometry(surfMesh);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    auto constraint = std::dynamic_pointer_cast<PbdAreaConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getStiffness(), 1.0e4);
    EXPECT_EQ(constraint->getParticles().size(), 3);
    EXPECT_EQ(constraint->getParticles()[0].second, 0);
    EXPECT_EQ(constraint->getParticles()[1].second, 1);
    EXPECT_EQ(constraint->getParticles()[2].second, 2);
}

///
/// \brief Test that the correct pbd constant density constraint was generated
///
TEST(imstkPbdConstraintFunctorTest, TestConstDensityConstraintGeneration)
{
    auto points   = std::make_shared<PointSet>();
    auto vertices = std::make_shared<VecDataArray<double, 3>>(4);
    (*vertices)[0] = Vec3d(0.0, 0.0, 0.0);
    (*vertices)[1] = Vec3d(1.0, 0.0, 0.0);
    (*vertices)[2] = Vec3d(0.0, 1.0, 0.0);
    (*vertices)[3] = Vec3d(0.0, 0.0, 1.0);
    points->initialize(vertices);

    // Create functor
    PbdConstantDensityConstraintFunctor constraintFunctor;
    constraintFunctor.setStiffness(1.0e4);
    constraintFunctor.setGeometry(points);

    // Fill container
    PbdConstraintContainer container;
    constraintFunctor(container);

    // Check that constraint got generated
    EXPECT_EQ(container.getConstraints().size(), 1);

    // Check that correct constraint type got generated
    auto constraint = std::dynamic_pointer_cast<PbdConstantDensityConstraint>(container.getConstraints()[0]);
    EXPECT_NE(constraint, nullptr);

    // Check constraint generated between correct elements and with correct values
    EXPECT_EQ(constraint->getParticles().size(), 0);
}