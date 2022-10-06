/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkCellPicker.h"
#include "imstkSphere.h"
#include "imstkPlane.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkCapsule.h"
#include "imstkLineMesh.h"
#include "imstkOrientedBox.h"
#include "imstkVecDataArray.h"
#include "imstkCDObjectFactory.h"

using namespace imstk;

TEST(imstkCellPickerTest, PickSurfaceMesh)
{
    auto mesh = std::make_shared<SurfaceMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(4);
        (*verticesPtr)[0] = Vec3d(0.5, 0.0, -0.5);
        (*verticesPtr)[1] = Vec3d(-0.5, 0.0, -0.5);
        (*verticesPtr)[2] = Vec3d(0.0, 0.0, 0.5);
        (*verticesPtr)[3] = Vec3d(0.5, 0.0, 0.5);
        auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(2);
        (*indicesPtr)[0] = Vec3i(0, 1, 2);
        (*indicesPtr)[1] = Vec3i(0, 2, 3);
        mesh->initialize(verticesPtr, indicesPtr);
    }

    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.5);

    CellPicker picker;
    picker.setCollisionDetection(CDObjectFactory::create("SurfaceMeshToSphereCD"));
    picker.setPickingGeometry(sphere);
    const std::vector<PickData>& pickData1 = picker.pick(mesh);
    EXPECT_EQ(2, pickData1.size());
    EXPECT_EQ(0, pickData1[0].cellId);
    EXPECT_EQ(1, pickData1[1].cellId);

    // Just graze the first triangle
    sphere->setPosition(Vec3d(0.0, -0.49, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(mesh);
    EXPECT_EQ(1, pickData2.size());
    EXPECT_EQ(0, pickData2[0].cellId);

    // Miss the triangles entirely
    sphere->setPosition(Vec3d(0.0, -0.51, 0.0));
    const std::vector<PickData>& pickData3 = picker.pick(mesh);
    EXPECT_EQ(0, pickData3.size());
}

TEST(imstkCellPickerTest, PickLineMesh)
{
    auto mesh = std::make_shared<LineMesh>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(-0.5, 0.0, 0.0);
        (*verticesPtr)[1] = Vec3d(0.5, 0.0, 0.0);
        (*verticesPtr)[2] = Vec3d(1.5, 0.0, 0.0);
        auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(2);
        (*indicesPtr)[0] = Vec2i(0, 1);
        (*indicesPtr)[1] = Vec2i(1, 2);
        mesh->initialize(verticesPtr, indicesPtr);
    }

    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 0.6);

    CellPicker picker;
    picker.setCollisionDetection(CDObjectFactory::create("LineMeshToSphereCD"));
    picker.setPickingGeometry(sphere);
    const std::vector<PickData>& pickData1 = picker.pick(mesh);
    EXPECT_EQ(2, pickData1.size());
    EXPECT_EQ(0, pickData1[0].cellId);
    EXPECT_EQ(1, pickData1[1].cellId);

    // Just graze the first segment
    sphere->setPosition(Vec3d(0.0, -0.59, 0.0));
    const std::vector<PickData>& pickData2 = picker.pick(mesh);
    EXPECT_EQ(1, pickData2.size());
    EXPECT_EQ(0, pickData2[0].cellId);

    // Miss the segments entirely
    sphere->setPosition(Vec3d(0.0, -10.0, 0.0));
    const std::vector<PickData>& pickData3 = picker.pick(mesh);
    EXPECT_EQ(0, pickData3.size());
}

TEST(imstkCellPickerTest, PickPointSet)
{
    auto mesh = std::make_shared<PointSet>();
    {
        auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
        (*verticesPtr)[0] = Vec3d(-0.5, 0.0, 0.0);
        (*verticesPtr)[1] = Vec3d(0.0, 0.0, 0.0);
        (*verticesPtr)[2] = Vec3d(0.5, 0.0, 0.0);
        mesh->initialize(verticesPtr);
    }

    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 1.5);

    // Select all
    CellPicker picker;
    picker.setCollisionDetection(CDObjectFactory::create("PointSetToSphereCD"));
    picker.setPickingGeometry(sphere);
    const std::vector<PickData>& pickData1 = picker.pick(mesh);
    EXPECT_EQ(3, pickData1.size());
    EXPECT_EQ(0, pickData1[0].cellId);
    EXPECT_EQ(1, pickData1[1].cellId);
    EXPECT_EQ(2, pickData1[2].cellId);

    // Only the center point
    sphere->setRadius(0.3);
    const std::vector<PickData>& pickData2 = picker.pick(mesh);
    EXPECT_EQ(1, pickData2.size());
    EXPECT_EQ(1, pickData2[0].cellId);

    // Miss the vertices entirely
    sphere->setPosition(Vec3d(0.0, -10.0, 0.0));
    const std::vector<PickData>& pickData3 = picker.pick(mesh);
    EXPECT_EQ(0, pickData3.size());
}