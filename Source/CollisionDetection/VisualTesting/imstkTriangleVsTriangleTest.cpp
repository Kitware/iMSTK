/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkVecDataArray.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Triangle collision
/// of the SurfaceMeshToSurfaceMeshCD method.
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, TriangleVsTriangle)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0.18, 1.08, 1.34);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0.011, 0.78, -0.63);

    auto                    triangleMesh1 = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices1(3);
    triangleVertices1[0] = Vec3d(0.1, 0.0, -0.5);
    triangleVertices1[1] = Vec3d(0.1, 0.0, 0.5);
    triangleVertices1[2] = Vec3d(-0.5, 0.0, 0.0);
    VecDataArray<int, 3> triangleIndices1(1);
    triangleIndices1[0] = Vec3i(0, 1, 2);
    triangleMesh1->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices1),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices1));
    m_cdGeom1 = triangleMesh1;

    auto                    triangleMesh2 = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices2(3);
    triangleVertices2[0] = Vec3d(-0.1, 0.5, 0.0);
    triangleVertices2[1] = Vec3d(-0.1, -0.5, 0.0);
    triangleVertices2[2] = Vec3d(0.5, 0.0, 0.0);
    VecDataArray<int, 3> triangleIndices2(1);
    triangleIndices2[0] = Vec3i(0, 1, 2);
    triangleMesh2->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices2),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices2));
    m_cdGeom2 = triangleMesh2;

    m_collisionMethod = std::make_shared<SurfaceMeshToSurfaceMeshCD>();
    m_collisionMethod->setInputGeometryA(triangleMesh1);
    m_collisionMethod->setInputGeometryB(triangleMesh2);
    m_collisionMethod->update();

    createScene();
    runFor(2.0);
}