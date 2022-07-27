/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <gtest/gtest.h>

#include <imstkGeometryAlgorithm.h>
#include <imstkSurfaceMesh.h>
#include <imstkSphere.h>

using namespace imstk;
namespace
{
class MockAlgorithm : public GeometryAlgorithm
{
public:

    bool areInputsValid() override
    {
        return GeometryAlgorithm::areInputsValid();
    }

    void requestUpdate() override
    {
    }
};

class EmptyAlgorithm : public MockAlgorithm
{
public:
    EmptyAlgorithm()
    {
        setNumInputPorts(4);
    }
};

class ExpectingAlgorithm : public MockAlgorithm
{
public:
    ExpectingAlgorithm()
    {
        setNumInputPorts(4);
        setRequiredInputType<SurfaceMesh>(1);
    }
};

class ExpectingOptional : public MockAlgorithm
{
public:
    ExpectingOptional()
    {
        setNumInputPorts(4);
        setOptionalInputType<SurfaceMesh>(1);
    }
};

class ExpectingAllKinds : public MockAlgorithm
{
public:
    ExpectingAllKinds()
    {
        setNumInputPorts(5);
        setRequiredInputType<SurfaceMesh>(1);
        setRequiredInputType<Sphere>(2);
        setOptionalInputType<SurfaceMesh>(3);
        setOptionalInputType<Sphere>(4);
    }
};
} // namespace

TEST(imstkGeometryAlgorithmTest, no_expectations)
{
    EmptyAlgorithm a;

    EXPECT_TRUE(a.areInputsValid());

    auto mesh = std::make_shared<SurfaceMesh>();
    a.setInput(mesh, 0);
    EXPECT_TRUE(a.areInputsValid());
}

TEST(imstkGeometryAlgorithmTest, required_only)
{
    ExpectingAlgorithm a;

    EXPECT_FALSE(a.areInputsValid());

    auto mesh = std::make_shared<SurfaceMesh>();
    a.setInput(mesh, 1);
    EXPECT_TRUE(a.areInputsValid());

    auto sphere = std::make_shared<Sphere>();
    a.setInput(sphere, 1);
    EXPECT_FALSE(a.areInputsValid());
}

TEST(imstkGeometryAlgorithmTest, optional_only)
{
    ExpectingOptional a;

    // optional is true when there isn't a value
    EXPECT_TRUE(a.areInputsValid());

    // should be true, expected is mesh
    auto mesh = std::make_shared<SurfaceMesh>();
    a.setInput(mesh, 1);
    EXPECT_TRUE(a.areInputsValid());

    // should be false ... mesh is expected
    auto sphere = std::make_shared<Sphere>();
    a.setInput(sphere, 1);
    EXPECT_FALSE(a.areInputsValid());
}

TEST(imstkGeometryAlgorithmTest, all_kinds)
{
    ExpectingAllKinds a;

    EXPECT_FALSE(a.areInputsValid());

    auto mesh   = std::make_shared<SurfaceMesh>();
    auto sphere = std::make_shared<Sphere>();
    a.setInput(mesh, 1);
    EXPECT_FALSE(a.areInputsValid());
    a.setInput(sphere, 2);
    EXPECT_TRUE(a.areInputsValid());

    a.setInput(mesh, 3);
    EXPECT_TRUE(a.areInputsValid());
    a.setInput(sphere, 4);
    EXPECT_TRUE(a.areInputsValid());

    a.setInput(sphere, 3);
    EXPECT_FALSE(a.areInputsValid());
    a.setInput(mesh, 4);
    EXPECT_FALSE(a.areInputsValid());
}