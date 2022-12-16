/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkEntity.h"
#include "imstkRenderMaterial.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Sphere collision
/// of the SurfaceMeshToSphereCD method
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, TriangleVsSphere)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0, 2.74, 2.69);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0, 0.71, -0.71);

    m_cdGeom1 = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 1.0);
    m_cdObj1->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    auto triangleMesh = std::make_shared<SurfaceMesh>();

    VecDataArray<double, 3> triangleVertices(3);
    triangleVertices[0] = Vec3d(-0.5, 0.9, -0.5);
    triangleVertices[1] = Vec3d(0.5, 0.9, -0.5);
    triangleVertices[2] = Vec3d(0.0, 0.9, 0.5);
    VecDataArray<int, 3> triangleIndices(1);
    triangleIndices[0] = Vec3i(0, 1, 2);
    triangleMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices));
    m_cdGeom2 = triangleMesh;

    m_collisionMethod = std::make_shared<SurfaceMeshToSphereCD>();
    m_collisionMethod->setInputGeometryA(m_cdGeom2);
    m_collisionMethod->setInputGeometryB(m_cdGeom1);
    m_collisionMethod->update();

    createScene();
    runFor(2.0);
}