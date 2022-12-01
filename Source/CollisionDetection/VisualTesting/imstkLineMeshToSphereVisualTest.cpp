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
#include "imstkLineMesh.h"
#include "imstkLineMeshToSphereCD.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief This test is used to investigate LineMesh Vs Sphere collision
/// of the LineMeshToSphereCD method
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, LineVsSphere)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0, 2.74, 2.69);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0, 0.71, -0.71);

    m_cdGeom1 = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.20);
    m_cdObj1->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    auto lineMesh = std::make_shared<LineMesh>();

    VecDataArray<double, 3> lineVertices(2);
    lineVertices[0] = Vec3d(-1.0, 0.1, 0.0);
    lineVertices[1] = Vec3d(1.0, 0.1, 0.0);

    VecDataArray<int, 2> lineIndices(1);
    lineIndices[0] = Vec2i(0, 1);
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(lineVertices),
        std::make_shared<VecDataArray<int, 2>>(lineIndices));
    m_cdGeom2 = lineMesh;

    m_collisionMethod = std::make_shared<LineMeshToSphereCD>();
    m_collisionMethod->setInputGeometryA(m_cdGeom2);
    m_collisionMethod->setInputGeometryB(m_cdGeom1);
    m_collisionMethod->update();

    createScene();
    runFor(2.0);
}