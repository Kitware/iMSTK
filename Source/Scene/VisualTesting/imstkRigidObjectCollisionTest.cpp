/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkCollisionDataDebugModel.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkPbdMethod.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdSystem.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneUtils.h"
#include "imstkSceneObject.h"
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

        ASSERT_NE(m_pbdObj, nullptr) << "Missing a pbdObj for PbdObjectCollisionTest";
        auto method   = m_pbdObj->getComponent<PbdMethod>();
        auto pointSet = std::dynamic_pointer_cast<PointSet>(method->getGeometry());
        m_prevBodyPos = method->getRigidPosition();
        m_scene->addSceneObject(m_pbdObj);

        ASSERT_NE(m_collidingGeometry, nullptr);
        m_cdObj = std::make_shared<SceneObject>("obj2");
        m_cdObj->setVisualGeometry(m_collidingGeometry);
        m_cdObj->addComponent<Collider>()->setGeometry(m_collidingGeometry);
        m_cdObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
        m_cdObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);
        m_scene->addSceneObject(m_cdObj);

        m_pbdCollision = std::make_shared<PbdObjectCollision>(m_pbdObj, m_cdObj, m_collisionName);
        m_pbdCollision->setFriction(m_friction);
        m_pbdCollision->setRestitution(m_restitution);
        m_scene->addInteraction(m_pbdCollision);

        // Debug geometry to visualize collision data
        m_cdDebugObject = m_pbdCollision->addComponent<CollisionDataDebugModel>();
        m_cdDebugObject->setPrintContacts(m_printContacts);

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
            [ = ](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                method->getPbdSystem()->getConfig()->m_dt =
                    m_sceneManager->getDt();
            });

        // Assert the vertices stay within bounds and below min displacement
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [ = ](Event*)
            {
                ASSERT_TRUE(assertBounds({ method->getRigidPosition() },
                    m_assertionBoundsMin, m_assertionBoundsMax));
                ASSERT_TRUE(assertMinDisplacement({ m_prevBodyPos },
                    { method->getRigidPosition() }, 0.01));
                m_prevBodyPos = method->getRigidPosition();
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<Entity>      m_pbdObj = nullptr;
    std::shared_ptr<SceneObject> m_cdObj  = nullptr;
    std::shared_ptr<Geometry>    m_collidingGeometry = nullptr;

    std::shared_ptr<PbdObjectCollision> m_pbdCollision = nullptr;
    std::string m_collisionName = "";
    double      m_friction      = 0.0;
    double      m_restitution   = 0.8;
    std::shared_ptr<CollisionDataDebugModel> m_cdDebugObject = nullptr;

    // For assertions
    Vec3d m_prevBodyPos;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;

    Vec3d m_assertionBoundsMin = Vec3d(-1.0, -1.0, -1.0);
    Vec3d m_assertionBoundsMax = Vec3d(1.0, 1.0, 1.0);
};

namespace
{
std::shared_ptr<PbdSystem>
makeSystem()
{
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_dt      = 0.01;
    pbdSystem->getConfig()->m_gravity = Vec3d(0.0, -9.81, 0.0);
    pbdSystem->getConfig()->m_linearDampingCoeff  = 0.01;
    pbdSystem->getConfig()->m_angularDampingCoeff = 0.01;
    return pbdSystem;
}
} // namespace anonymous

///
/// \brief Test CapsuleToCapsuleCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, RigidObj_CapsuleToCapsuleCD)
{
    // Setup the rigid object
    {
        auto colGeom = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.05, 0.2);

        // Setup the Model
        auto pbdSystem = makeSystem();
        pbdSystem->getConfig()->m_doPartitioning = false;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.01);
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Capsule>(Vec3d(0.0, -0.5, 0.0), 0.2, 1.0,
        Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.2, 0.0).normalized()));
    m_collidingGeometry = implicitGeom;

    m_collisionName = "CapsuleToCapsuleCD";
    m_friction      = 0.01;
    m_restitution   = 0.9;

    m_assertionBoundsMin = Vec3d(-20.0, -20.0, -20.0);
    m_assertionBoundsMax = Vec3d(20.0, 20.0, 20.0);

    connect<KeyEvent>(m_viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
        [this](KeyEvent*)
        {
            m_pbdObj->getComponent<Collider>()->getGeometry()->print();
            m_collidingGeometry->print();
        });

    createScene();
    runFor(5.0);
}

///
/// \brief Test SurfaceMeshToCapsuleCD with RigidObjectCollision
///
TEST_F(RigidObjectCollisionTest, DISABLED_RigidObj_SurfaceMeshToCapsuleCD)
{
    // Setup the rigid object
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
        auto pbdSystem = makeSystem();

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.00005);
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
TEST_F(RigidObjectCollisionTest, RigidObj_SphereToSphereCD)
{
    // Setup the rigid object
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto pbdSystem = makeSystem();
        pbdSystem->getConfig()->m_dt = 0.001;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
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
TEST_F(RigidObjectCollisionTest, RigidObj_UnidirectionalPlaneToSphereCD)
{
    // Setup the rigid object
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto pbdSystem = makeSystem();
        pbdSystem->getConfig()->m_dt = 0.001;

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
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
TEST_F(RigidObjectCollisionTest, RigidObj_BidirectionalPlaneToSphereCD)
{
    // Setup the rigid object
    {
        // Setup the Geometry
        auto colGeom = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), 0.05);

        // Setup the Model
        auto pbdSystem = makeSystem();
        pbdSystem->getConfig()->m_dt      = 0.001;
        pbdSystem->getConfig()->m_gravity = Vec3d(0.0, 9.81, 0.0);

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setColor(Color(0.77, 0.53, 0.34));

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
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
TEST_F(RigidObjectCollisionTest, RigidObj_PointSetToCapsuleCD)
{
    // Setup the rigid object
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
        auto pbdSystem = makeSystem();

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
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
TEST_F(RigidObjectCollisionTest, RigidObj_PointSetToSphereCD)
{
    // Setup the rigid object
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
        auto pbdSystem = makeSystem();

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
    }

    // Setup the geometry
    auto implicitGeom = std::make_shared<Sphere>(Vec3d(0.01, -0.3, 0.0), 0.2);
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
TEST_F(RigidObjectCollisionTest, RigidObj_PointSetToPlaneCD)
{
    // Setup the rigid object
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
        auto pbdSystem = makeSystem();

        // Setup the VisualModel
        auto material = std::make_shared<RenderMaterial>();
        material->setBackFaceCulling(false);
        material->setDisplayMode(RenderMaterial::DisplayMode::Points);
        material->setColor(Color(0.77, 0.53, 0.34));
        material->setEdgeColor(Color(0.87, 0.63, 0.44));
        material->setPointSize(20.0);

        m_pbdObj = SceneUtils::makePbdEntity("obj1", colGeom, pbdSystem);
        m_pbdObj->getComponent<VisualModel>()->setRenderMaterial(material);
        m_pbdObj->getComponent<PbdMethod>()->setRigid(Vec3d::Zero(), 0.1, Quatd::Identity(), Mat3d::Identity() * 0.005);
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