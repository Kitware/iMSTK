/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionDetectionVisualTest.h"
#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDataDebugModel.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometry.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

void
CollisionDetectionVisualTest::SetUp()
{
    VisualTest::SetUp();

    auto defaultMaterial = std::make_shared<RenderMaterial>();
    defaultMaterial->setOpacity(0.5);
    defaultMaterial->setBackFaceCulling(false);
    defaultMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    m_cdObj1 = std::make_shared<CollidingObject>("cdObj1");
    auto model1 = m_cdObj1->addComponent<VisualModel>();
    model1->setRenderMaterial(defaultMaterial);

    m_cdObj2 = std::make_shared<CollidingObject>("cdObj2");
    auto model2 = m_cdObj2->addComponent<VisualModel>();
    model2->setRenderMaterial(defaultMaterial);
}

void
CollisionDetectionVisualTest::createScene()
{
    // Setup the scene
    m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
    if (m_camera != nullptr)
    {
        *m_scene->getActiveCamera() = *m_camera;
    }

    ASSERT_NE(m_cdGeom1, nullptr) <<
        "Missing a m_cdGeom1 Geometry for CollisionDetectionVisualTest";
    ASSERT_NE(m_cdObj1, nullptr) <<
        "Missing a m_cdObj1 CollidingObject for CollisionDetectionVisualTest";
    m_cdObj1->getVisualModel(0)->setGeometry(m_cdGeom1);
    m_cdObj1->setCollidingGeometry(m_cdGeom1);
    m_scene->addSceneObject(m_cdObj1);

    ASSERT_NE(m_cdGeom2, nullptr) <<
        "Missing a m_cdGeom2 Geometry for CollisionDetectionVisualTest";
    ASSERT_NE(m_cdObj2, nullptr) <<
        "Missing a m_cdObj2 CollidingObject for CollisionDetectionVisualTest";
    m_cdObj2->getVisualModel(0)->setGeometry(m_cdGeom2);
    m_cdObj2->setCollidingGeometry(m_cdGeom2);
    m_scene->addSceneObject(m_cdObj2);

    ASSERT_NE(m_collisionMethod, nullptr) <<
        "Missing a m_collisionMethod CollisionDetectionAlgorithm for CollisionDetectionVisualTest";

    // Debug geometry to visualize collision data
    auto cdDebugObj = std::make_shared<Entity>();
    m_cdDebugObject = cdDebugObj->addComponent<CollisionDataDebugModel>();
    m_cdDebugObject->setInputCD(m_collisionMethod->getCollisionData());
    m_cdDebugObject->setPrintContacts(m_printContacts);
    m_scene->addSceneObject(cdDebugObj);

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [this](KeyEvent* e)
        {
            bool moved = false;
            // Transform & immediately update post transform geometry
            if (m_geom1Moveable && moveGeometryByKey(e->m_key, m_cdGeom1))
            {
                moved = true;
            }
            if (m_geom2Moveable && moveGeometryByKey(e->m_key, m_cdGeom2))
            {
                moved = true;
            }
            if (moved)
            {
                // Recompute the CD & update debug CD object
                m_collisionMethod->update();
                m_cdDebugObject->debugUpdate();
            }
        });
    connect<Event>(m_driver, &SimulationManager::starting,
        [&](Event*)
        {
            m_cdDebugObject->debugUpdate();
        });

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    m_scene->addLight("Light", light);
}

bool
CollisionDetectionVisualTest::moveGeometryByKey(const unsigned char       key,
                                                std::shared_ptr<Geometry> geom)
{
    const Mat4d  initTransform = geom->getTransform();
    const double s = 0.05;
    if (key == 'i')
    {
        geom->translate(Vec3d(0.0, 0.0, 1.0) * s);
    }
    else if (key == 'k')
    {
        geom->translate(Vec3d(0.0, 0.0, -1.0) * s);
    }
    else if (key == 'j')
    {
        geom->translate(Vec3d(-1.0, 0.0, 0.0) * s);
    }
    else if (key == 'l')
    {
        geom->translate(Vec3d(1.0, 0.0, 0.0) * s);
    }
    else if (key == 'u')
    {
        geom->translate(Vec3d(0.0, -1.0, 0.0) * s);
    }
    else if (key == 'o')
    {
        geom->translate(Vec3d(0.0, 1.0, 0.0) * s);
    }
    else if (key == '1')
    {
        geom->rotate(Vec3d(0.0, 0.0, 1.0), 0.1);
    }
    else if (key == '2')
    {
        geom->rotate(Vec3d(0.0, 0.0, 1.0), -0.1);
    }
    else if (key == '3')
    {
        geom->rotate(Vec3d(0.0, 1.0, 0.0), 0.1);
    }
    else if (key == '4')
    {
        geom->rotate(Vec3d(0.0, 1.0, 0.0), -0.1);
    }

    if (initTransform != geom->getTransform())
    {
        geom->postModified();
        geom->updatePostTransformData();
        return true;
    }
    else
    {
        return false;
    }
}