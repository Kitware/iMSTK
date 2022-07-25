/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkClosedSurfaceMeshToMeshCD.h"
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkGeometryUtilities.h"
#include "imstkOrientedBox.h"
#include "imstkSurfaceMesh.h"

using namespace imstk;

///
/// \brief This test is used to investigate Triangle Vs Triangle collision
/// of the ClosedSurfaceMeshToMeshCD method.
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, BoxVsBoxSurfaceMesh)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setFocalPoint(-0.0366287, 0.420204, 0.474284);
    m_camera->setPosition(-2.60143, 1.23713, 2.42823);
    m_camera->setViewUp(0.216266, 0.968787, -0.121162);

    // Create a box mesh
    auto box1 = std::make_shared<OrientedBox>(
        Vec3d::Zero(), Vec3d(0.5, 0.5, 0.5), Quatd::Identity());
    auto box2 = std::make_shared<OrientedBox>(
        Vec3d::Zero(), Vec3d(0.4, 0.4, 0.4), Quatd::Identity());

    m_cdGeom1 = GeometryUtils::toSurfaceMesh(box1);
    m_cdGeom2 = GeometryUtils::toSurfaceMesh(box2);
    m_cdGeom2->rotate(Vec3d(0.0, 0.0, 1.0), PI_2 * 0.5);
    m_cdGeom2->rotate(Vec3d(1.0, 0.0, 0.0), PI_2 * 0.5);
    m_cdGeom2->translate(Vec3d(0.0, 0.8, 0.8));

    auto cd = std::make_shared<ClosedSurfaceMeshToMeshCD>();
    cd->setInputGeometryA(m_cdGeom1);
    cd->setInputGeometryB(m_cdGeom2);
    cd->setGenerateEdgeEdgeContacts(true);
    cd->update();
    m_collisionMethod = cd;

    createScene();
    runFor(2.0);
}