/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkEntity.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This test is used to investigate Capsule Vs Capsule collision
/// of the CapsuleToCapsuleCD method
/// It displays the collision data, and allows users to investigate various cases
///
TEST_F(CollisionDetectionVisualTest, CapsuleVsSurfaceMesh)
{
    m_camera = std::make_shared<Camera>();
    m_camera->setPosition(0, 2.74, 2.69);
    m_camera->setFocalPoint(0.0, 0.0, 0.0);
    m_camera->setViewUp(0, 0.71, -0.71);

    auto                    triangleMesh = std::make_shared<SurfaceMesh>();
    VecDataArray<double, 3> triangleVertices(3);
    triangleVertices[0] = Vec3d(0.5, 0.0, -1.0 / 3.0);
    triangleVertices[1] = Vec3d(-0.5, 0.1, -1.0 / 3.0);
    triangleVertices[2] = Vec3d(0.0, 0.0, 2.0 / 3.0);

    VecDataArray<int, 3> triangleIndices(1);
    triangleIndices[0] = Vec3i(0, 1, 2);
    triangleMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(triangleVertices),
        std::make_shared<VecDataArray<int, 3>>(triangleIndices));
    m_cdGeom1 = triangleMesh;

    m_cdGeom2 = std::make_shared<Capsule>(Vec3d(0.0, 0.1, 0.0), 0.1, 0.5);
    m_cdObj2->getComponent<VisualModel>()->getRenderMaterial()->setDisplayMode(
        RenderMaterial::DisplayMode::Surface);

    m_collisionMethod = std::make_shared<SurfaceMeshToCapsuleCD>();
    m_collisionMethod->setInputGeometryA(m_cdGeom1);
    m_collisionMethod->setInputGeometryB(m_cdGeom2); // capsule
    m_collisionMethod->update();

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [this](KeyEvent* e)
        {
            if (e->m_key == '5')
            {
                auto capsule1 = std::dynamic_pointer_cast<Capsule>(m_cdGeom2);
                capsule1->setRadius(capsule1->getRadius() * 1.1);
            }
            else if (e->m_key == '6')
            {
                auto capsule1 = std::dynamic_pointer_cast<Capsule>(m_cdGeom2);
                capsule1->setRadius(capsule1->getRadius() * 0.9);
            }
        });

    createScene();
    runFor(2.0);
}