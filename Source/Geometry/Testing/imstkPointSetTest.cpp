/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace imstk;

namespace
{
auto floatArray3 = std::make_shared<VecDataArray<float, 3>>(VecDataArray<float, 3>({
        Vec3f{ 0.0f, 0.1f, 0.2f },
        Vec3f{ 1.0f, 1.1f, 1.2f },
        Vec3f{ 2.0f, 2.1f, 2.2f },
}));

auto doubleArray3 = std::make_shared<VecDataArray<double, 3>>(VecDataArray<double, 3>({
        Vec3d{ 0.0, 0.1, 0.2 },
        Vec3d{ 1.0, 1.1, 1.2 },
        Vec3d{ 2.0, 2.1, 2.2 },
        Vec3d{ 3.0, 3.1, 3.2 },
 }));

auto floatArray2 = std::make_shared<VecDataArray<float, 2>>(VecDataArray<float, 2>({
        Vec2f{ 0.0f, 0.1f },
        Vec2f{ 1.0f, 1.1f },
        Vec2f{ 2.0f, 2.1f },
    }));

auto doubleArray2 = std::make_shared<VecDataArray<double, 2>>(VecDataArray<double, 2>({
        Vec2d{ 0.0, 0.1 },
        Vec2d{ 1.0, 1.1 },
        Vec2d{ 2.0, 2.1 },
    }));

std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes = {
    { "float3", floatArray3 }, { "double3", doubleArray3 },
    { "float2", floatArray2 }, { "double2", doubleArray2 }
};
} // namespace

TEST(imstkPointSetTest, VertexNormalAttributes)
{
    PointSet p;
    p.setVertexAttributes(attributes);
    p.setVertexNormals("double3");
    EXPECT_EQ(doubleArray3, p.getVertexNormals());

    // Normals want doubles, test with floats
    p.setVertexNormals("float3");
    auto normals = p.getVertexNormals();
    ASSERT_NE(nullptr, normals);
    EXPECT_NE(floatArray3->getVoidPointer(), normals->getVoidPointer());
    EXPECT_EQ(3, normals->size());
    for (int i = 0; i < normals->size(); ++i)
    {
        EXPECT_TRUE((*floatArray3)[i].cast<double>().isApprox((*normals)[i]));
    }

    // This could work we'd need to make the DataArray a little bit more standards compliant
    // EXPECT_THAT(*normals, ElementsAreArray(doubleArray3->begin(), doubleArray3->end()));
    // HS 2021-apr-04 Death tests don't work with the current infrastructure
    //ASSERT_DEATH(p.setVertexNormals("float2"), ".*");
}

TEST(imstkPointSetTest, VertexTangentAttributes)
{
    PointSet p;
    p.setVertexAttributes(attributes);
    p.setVertexTangents("float3");
    EXPECT_EQ(floatArray3, p.getVertexTangents());

    // Tangents want floats, test with doubles
    p.setVertexTangents("double3");
    auto tangents = p.getVertexTangents();
    ASSERT_NE(nullptr, tangents);
    EXPECT_NE(doubleArray3->getVoidPointer(), tangents->getVoidPointer());
    EXPECT_EQ(4, tangents->size());
    for (int i = 0; i < tangents->size(); ++i)
    {
        EXPECT_TRUE((*doubleArray3)[i].cast<float>().isApprox((*tangents)[i]));
    }

    // HS 2021-apr-04 Death tests don't work with the current infrastructure
    //ASSERT_DEATH(p.setVertexTangents("float2"), ".*");
}

TEST(imstkPointSetTest, VertexTexCoordAttributes)
{
    PointSet p;
    p.setVertexAttributes(attributes);
    p.setVertexTCoords("float2");
    EXPECT_EQ(floatArray2, p.getVertexTCoords());

    // TexCoords wants floats, test with doubles
    p.setVertexTCoords("double2");
    auto tcoords = p.getVertexTCoords();
    ASSERT_NE(nullptr, tcoords);
    EXPECT_NE(doubleArray2->getVoidPointer(), tcoords->getVoidPointer());
    for (int i = 0; i < tcoords->size(); ++i)
    {
        EXPECT_TRUE((*doubleArray2)[i].cast<float>().isApprox((*tcoords)[i]));
    }

    // HS 2021-apr-04 Death tests don't work with the current infrastructure
    //ASSERT_DEATH(p.setVertexTangents("float2"), ".*");
}

TEST(imstkPointSetTest, VertexNormalTransform)
{
    PointSet p;
    p.setVertexAttributes(attributes);
    p.setVertexNormals("double3");

    auto vertexCopy = std::make_shared<VecDataArray<double, 3>>(VecDataArray<double, 3>({
        Vec3d{ 0.0, 0.1, 0.2 },
        Vec3d{ 1.0, 1.1, 1.2 },
        Vec3d{ 2.0, 2.1, 2.2 },
        Vec3d{ 3.0, 3.1, 3.2 },
        }));

    // Roatate mesh
    auto   axis = Vec3d(0.0, 0.0, 1.0);
    double rad  = -PI / 16;
    p.rotate(axis, rad, Geometry::TransformType::ApplyToData);

    auto rotation = Rotd(rad, axis.normalized()).toRotationMatrix();

    std::shared_ptr<VecDataArray<double, 3>> normalsPtr = p.getVertexNormals();
    VecDataArray<double, 3>&                 normals    = *normalsPtr;

    for (int vertId = 0; vertId < p.getNumVertices(); vertId++)
    {
        EXPECT_EQ(rotation * (*vertexCopy)[vertId].normalized(), normals[vertId]);
    }
}

TEST(imstkPointSetTest, VertexTangentsTransform)
{
    PointSet p;
    p.setVertexAttributes(attributes);
    p.setVertexTangents("float3");

    auto vertexCopy = std::make_shared<VecDataArray<float, 3>>(VecDataArray<float, 3>({
        Vec3f{ 0.0, 0.1, 0.2 },
        Vec3f{ 1.0, 1.1, 1.2 },
        Vec3f{ 2.0, 2.1, 2.2 },
        Vec3f{ 3.0, 3.1, 3.2 },
        }));

    // Roatate mesh
    auto   axis = Vec3d(0.0, 0.0, 1.0);
    double rad  = -PI / 16;
    p.rotate(axis, rad, Geometry::TransformType::ApplyToData);

    auto rotationD = Rotd(rad, axis.normalized()).toRotationMatrix();
    auto rotationF = rotationD.cast<float>();

    std::shared_ptr<VecDataArray<float, 3>> tangentsPtr = p.getVertexTangents();
    VecDataArray<float, 3>&                 tangents    = *tangentsPtr;

    for (int vertId = 0; vertId < p.getNumVertices(); vertId++)
    {
        EXPECT_EQ(rotationF * (*vertexCopy)[vertId].normalized(), tangents[vertId]);
    }
}