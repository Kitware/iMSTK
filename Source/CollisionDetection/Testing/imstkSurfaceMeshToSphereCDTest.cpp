/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkSurfaceMeshToSphereCDTest, IntersectionTestAB_Vertex)
{
    // Vertex-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 1.0);

    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.0, 0.5, 0.0);
    (*verticesPtr)[1] = Vec3d(0.5, 0.5, -2.0);
    (*verticesPtr)[2] = Vec3d(-0.5, 0.5, -2.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    // manually test for collision
    SurfaceMeshToSphereCD m_colDetect;
    m_colDetect.setInput(surfMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::PointIndexDirection, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);
}

TEST(imstkSurfaceMeshToSphereCDTest, IntersectionTestAB_Edge)
{
    // Triangle edge-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 1.0);

    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.5, 0.5, 0.0);
    (*verticesPtr)[1] = Vec3d(-0.5, 0.5, 0.0);
    (*verticesPtr)[2] = Vec3d(0.0, 0.5, -2.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    SurfaceMeshToSphereCD m_colDetect;
    m_colDetect.setInput(surfMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_EDGE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

TEST(imstkSurfaceMeshToSphereCDTest, IntersectionTestAB_Face)
{
    // Triangle face-to-sphere case
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 1.0);

    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.0, 0.5, 2.0);
    (*verticesPtr)[1] = Vec3d(0.5, 0.5, -2.0);
    (*verticesPtr)[2] = Vec3d(-0.5, 0.5, -2.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    SurfaceMeshToSphereCD m_colDetect;
    m_colDetect.setInput(surfMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(1, colData->elementsA.size());
    EXPECT_EQ(1, colData->elementsB.size());

    EXPECT_EQ(CollisionElementType::CellIndex, colData->elementsA[0].m_type);
    EXPECT_EQ(CollisionElementType::PointDirection, colData->elementsB[0].m_type);

    EXPECT_EQ(IMSTK_TRIANGLE, colData->elementsA[0].m_element.m_CellIndexElement.cellType);
}

TEST(imstkSurfaceMeshToSphereCDTest, NonIntersectionTestAB)
{
    // No intersection
    auto sphere = std::make_shared<Sphere>(Vec3d::Zero(), 1.0);

    auto surfMesh    = std::make_shared<SurfaceMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(3);
    (*verticesPtr)[0] = Vec3d(0.0, 1.1, 2.0);
    (*verticesPtr)[1] = Vec3d(0.5, 1.1, -2.0);
    (*verticesPtr)[2] = Vec3d(-0.5, 1.1, -2.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 3>>(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    surfMesh->initialize(verticesPtr, indicesPtr);

    SurfaceMeshToSphereCD m_colDetect;
    m_colDetect.setInput(surfMesh, 0);
    m_colDetect.setInput(sphere, 1);
    m_colDetect.setGenerateCD(true, true);
    m_colDetect.update();

    std::shared_ptr<CollisionData> colData = m_colDetect.getCollisionData();

    EXPECT_EQ(0, colData->elementsA.size());
    EXPECT_EQ(0, colData->elementsB.size());
}