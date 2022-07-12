/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollisionDataDebugObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectCollision.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTestingUtils.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"
#include "imstkVTKViewer.h"

using namespace imstk;

class RigidObjectCollisionTest : public VisualTest
{
public:
    RigidObjectCollisionTest()
    {
        m_useStdOut = true;
    }

public:
    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());
        m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        ASSERT_NE(m_rbdObj, nullptr) << "Missing a pbdObj for PbdObjectCollisionTest";
        auto pointSet = std::dynamic_pointer_cast<PointSet>(m_rbdObj->getPhysicsGeometry());
        m_prevBodyPos = m_rbdObj->getRigidBody()->m_initPos;
        m_scene->addSceneObject(m_rbdObj);

        ASSERT_NE(m_collidingGeometry, nullptr);
        m_cdObj = std::make_shared<CollidingObject>("obj2");
        m_cdObj->setVisualGeometry(m_collidingGeometry);
        m_cdObj->setCollidingGeometry(m_collidingGeometry);
        m_cdObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
        m_cdObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
        m_scene->addSceneObject(m_cdObj);

        m_rbdCollision = std::make_shared<RigidObjectCollision>(m_rbdObj, m_cdObj, m_collisionName);
        m_rbdCollision->setFriction(m_friction);
        m_rbdCollision->setBaumgarteStabilization(m_beta);
        m_scene->addInteraction(m_rbdCollision);

        // Debug geometry to visualize collision data
        m_cdDebugObject = std::make_shared<CollisionDataDebugObject>();
        m_cdDebugObject->setInputCD(m_rbdCollision->getCollisionDetection()->getCollisionData());
        m_cdDebugObject->setPrintContacts(m_printContacts);
        m_scene->addSceneObject(m_cdDebugObject);

        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                m_cdDebugObject->debugUpdate();
                if (m_pauseOnContact && !m_timerPaused
                    && m_cdDebugObject->getInputCD()->elementsA.size() > 0)
                {
                    m_timerPaused = true;
                    m_sceneManager->pause();
                }
            });
        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_rbdObj->getRigidBodyModel2()->getConfig()->m_dt =
                    m_sceneManager->getDt() * 0.25;
            });

        // Assert the vertices stay within bounds and below min displacement
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                ASSERT_TRUE(assertBounds({ *m_rbdObj->getRigidBody()->m_pos },
                    m_assertionBoundsMin, m_assertionBoundsMax));
                ASSERT_TRUE(assertMinDisplacement({ m_prevBodyPos },
                    { *m_rbdObj->getRigidBody()->m_pos }, 0.01));
                m_prevBodyPos = *m_rbdObj->getRigidBody()->m_pos;
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<RigidObject2>    m_rbdObj     = nullptr;
    std::shared_ptr<CollidingObject> m_cdObj      = nullptr;
    std::shared_ptr<Geometry> m_collidingGeometry = nullptr;

    std::shared_ptr<RigidObjectCollision> m_rbdCollision = nullptr;
    std::string m_collisionName = "";
    double      m_friction      = 0.0;
    double      m_beta = 0.01;
    std::shared_ptr<CollisionDataDebugObject> m_cdDebugObject = nullptr;

    // For assertions
    Vec3d m_prevBodyPos;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;

    Vec3d m_assertionBoundsMin = Vec3d(-1.0, -1.0, -1.0);
    Vec3d m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);
};

///
/// \brief Test CapsuleToCapsuleCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_CapsuleToCapsuleCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        auto colGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.05, 0.2);

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.98;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.98;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.00005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>(Vec3d(0.0, -0.5, 0.0), 0.2, 1.0,
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.2, 0.0).normalized()));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "CapsuleToCapsuleCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [this](KeyEvent*)
        {
            m_rbdObj->getCollidingGeometry()->print();
            m_collidingGeometry->print();
        });

    createScene();
    runFor(2.0);
}

///
/// \brief Test SurfaceMeshToCapsuleCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, DISABLED_RbdObj_SurfaceMeshToCapsuleCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto                    colGeom = std::make_shared<SurfaceMesh>();
        VecDataArray<double, 3> verts(3);
        verts[0] = Vec3d(0.0, 0.0, -0.1);
        verts[1] = Vec3d(-0.1, 0.0, 0.1);
        verts[2] = Vec3d(0.1, 0.0, 0.1);
        VecDataArray<int, 3> cells = { Vec3i(0, 1, 2) };
        colGeom->initialize(
            std::make_shared<VecDataArray<double, 3>>(verts),
            std::make_shared<VecDataArray<int, 3>>(cells));

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>(Vec3d(0.01, -0.5, 0.0), 0.2, 2.0,
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, -0.3, 0.0)));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "SurfaceMeshToCapsuleCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test SphereToSphereCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_SphereToSphereCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry (bump it over 0.01 so it slides off)
    auto implicitGeom = std::make_shared<Sphere>(Vec3d(0.01, -0.2, 0.0), 0.1);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "SphereToSphereCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test UnidirectionalPlaneToSphereCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_UnidirectionalPlaneToSphereCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "UnidirectionalPlaneToSphereCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test BidirectionalPlaneToSphereCD with RigidObjectCollision
/// Sphere and gravity going up to test bidirectionality
///
TEST_F(RigidObjectCollisionTest, RbdObj_BidirectionalPlaneToSphereCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, 9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, 0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "BidirectionalPlaneToSphereCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    m_scene->getActiveCamera()->setPosition(0.0, -0.4, -0.7);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);
    runFor(2.0);
}

///
/// \brief Test PointSetToSphereCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_PointSetToCapsuleCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto                    colGeom = std::make_shared<PointSet>();
        VecDataArray<double, 3> verts(8);
        double                  size = 0.05;
        verts[0] = Vec3d(-size, -size, -size);
        verts[1] = Vec3d(-size, -size, size);
        verts[2] = Vec3d(-size, size, -size);
        verts[3] = Vec3d(-size, size, size);
        verts[4] = Vec3d(size, -size, -size);
        verts[5] = Vec3d(size, -size, size);
        verts[6] = Vec3d(size, size, -size);
        verts[7] = Vec3d(size, size, size);
        colGeom->initialize(std::make_shared<VecDataArray<double, 3>>(verts));

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>(Vec3d(0.01, -0.3, 0.0), 0.2, 2.0,
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, -0.3, 0.0)));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToCapsuleCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToSphereCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_PointSetToSphereCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto                    colGeom = std::make_shared<PointSet>();
        VecDataArray<double, 3> verts(8);
        double                  size = 0.05;
        verts[0] = Vec3d(-size, -size, -size);
        verts[1] = Vec3d(-size, -size, size);
        verts[2] = Vec3d(-size, size, -size);
        verts[3] = Vec3d(-size, size, size);
        verts[4] = Vec3d(size, -size, -size);
        verts[5] = Vec3d(size, -size, size);
        verts[6] = Vec3d(size, size, -size);
        verts[7] = Vec3d(size, size, size);
        colGeom->initialize(std::make_shared<VecDataArray<double, 3>>(verts));

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Sphere>(Vec3d(0.01, -0.4, 0.0), 0.2);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToSphereCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    createScene();
    runFor(2.0);
}

///
/// \brief Test PointSetToPlaneCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RbdObj_PointSetToPlaneCD)
{
    // Setup the rigid object
    m_rbdObj = std::make_shared<RigidObject2>("obj1");
    {
        // Setup the Geometry
        auto                    colGeom = std::make_shared<PointSet>();
        VecDataArray<double, 3> verts(8);
        double                  size = 0.05;
        verts[0] = Vec3d(-size, -size, -size);
        verts[1] = Vec3d(-size, -size, size);
        verts[2] = Vec3d(-size, size, -size);
        verts[3] = Vec3d(-size, size, size);
        verts[4] = Vec3d(size, -size, -size);
        verts[5] = Vec3d(size, -size, size);
        verts[6] = Vec3d(size, size, -size);
        verts[7] = Vec3d(size, size, size);
        colGeom->initialize(std::make_shared<VecDataArray<double, 3>>(verts));

        // Setup the Model
        auto rbdModel = std::make_shared<RigidBodyModel2>();
        rbdModel->getConfig()->m_dt      = 0.001;
        rbdModel->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);
        rbdModel->getConfig()->m_velocityDamping = 0.99;
        rbdModel->getConfig()->m_angularVelocityDamping = 0.99;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_rbdObj->setVisualGeometry(colGeom);
        m_rbdObj->getVisualModel(0)->setRenderMaterial(material);
        m_rbdObj->setPhysicsGeometry(colGeom);
        m_rbdObj->setCollidingGeometry(colGeom);
        m_rbdObj->setDynamicalModel(rbdModel);
        m_rbdObj->getRigidBody()->m_mass = 0.1;
        m_rbdObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 0.005;
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;

    m_assertionBoundsMin = Vec3d(-1.0, -0.15, -1.0);
    m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);

    createScene();
    runFor(2.0);
}