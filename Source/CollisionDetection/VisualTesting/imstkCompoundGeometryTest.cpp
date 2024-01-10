/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkCapsuleToCapsuleCD.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDetectionVisualTest.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkPbdObjectCollision.h"
#include "imstkDirectionalLight.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkCompoundGeometry.h"
#include "imstkLogger.h"
#include "imstkLoggerG3.h"

using namespace imstk;

std::shared_ptr<PbdObject>
makeCompoundObject(std::shared_ptr<PbdModel> pbdModel, Vec3d startingPos)
{
    auto rigidPbdObj = std::make_shared<PbdObject>("compoundRigidPbdObject");

    double radius = 0.025;

    Eigen::Affine3d rot1 =
        Eigen::Affine3d(Eigen::AngleAxisd(1.51, Eigen::Vector3d(0, 0, 1)));

    Eigen::Affine3d rot2 =
        Eigen::Affine3d(Eigen::AngleAxisd(1.51, Eigen::Vector3d(1, 0, 0)));

    Eigen::Affine3d trans1 = Eigen::Affine3d(Eigen::Translation3d(0.05, 0.0, 0.0));
    Eigen::Affine3d trans2 = Eigen::Affine3d(Eigen::Translation3d(0.0, 0.05, 0.0));

    auto  rigidGeom = std::make_shared<CompoundGeometry>();
    auto  geom1     = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.01, 0.05);
    auto  geom2     = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.01, 0.05);
    auto  geom3     = std::make_shared<Capsule>(Vec3d(0.0, 0.0, 0.0), 0.01, 0.05);
    Mat4d m1 = rot1.matrix();
    Mat4d m2 = rot2.matrix();

//      auto geom1 = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), radius);
//      auto geom2 = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), radius);
//      auto geom3 = std::make_shared<Sphere>(Vec3d(0.0, 0.0, 0.0), radius);
//      Mat4d m1 = rot1.matrix();
//      Mat4d m2 = rot2.matrix();

    rigidGeom->add(geom1);
    rigidGeom->add(geom2);
    rigidGeom->setLocalTransform(1, m1);
    rigidGeom->add(geom3);
    rigidGeom->setLocalTransform(2, m2);

    rigidPbdObj->setVisualGeometry(geom1);
    rigidPbdObj->setCollidingGeometry(rigidGeom);
    rigidPbdObj->setPhysicsGeometry(rigidGeom);

    {
        auto visuals = std::make_shared<VisualModel>();
        visuals->setGeometry(geom2);
        rigidPbdObj->addVisualModel(visuals);
    }
    {
        auto visuals = std::make_shared<VisualModel>();
        visuals->setGeometry(geom3);
        rigidPbdObj->addVisualModel(visuals);
    }

    // Setup material
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.0, 0.0));
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    rigidPbdObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    rigidPbdObj->setDynamicalModel(pbdModel);

    // Setup body
    const Quatd orientation = Quatd::FromTwoVectors(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 1.0, 1.0).normalized());
    rigidPbdObj->getPbdBody()->setRigid(startingPos, 1.0, orientation, Mat3d::Identity() * 0.01);
    return rigidPbdObj;
}

std::shared_ptr<PbdObject>
makeTestSphereObject(std::shared_ptr<PbdModel> pbdModel, Vec3d startingPos)
{
    // Make a pbd rigid body sphere
    auto sphereObj  = std::make_shared<PbdObject>("SimpleSphere");
    auto sphereGeom = std::make_shared<Sphere>(Vec3d::Zero(), 0.02);
    sphereObj->setVisualGeometry(sphereGeom);
    sphereObj->setCollidingGeometry(sphereGeom);
    sphereObj->setPhysicsGeometry(sphereGeom);
    sphereObj->setDynamicalModel(pbdModel);
    sphereObj->getPbdBody()->setRigid(
                startingPos,
                1.0,
                Quatd::Identity(),
                Mat3d::Identity() * 0.01);
    sphereObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);

    return sphereObj;
}

class CompoundCollisionVisualTest : public VisualTest
{
};

TEST_F(CompoundCollisionVisualTest, CompoundCapsule)
{
    // Setup the scene
        m_scene = std::make_shared<Scene>(::testing::UnitTest::GetInstance()->current_test_info()->name());

    // Setup a scene
        auto scene = std::make_shared<Scene>("PbdRigidBody");
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setPosition(-1, 1, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        auto pbdModel  = std::make_shared<PbdModel>();
        auto pbdConfig = std::make_shared<PbdModelConfig>();
    // Slightly larger gravity to compensate damping
        pbdConfig->m_gravity    = Vec3d(0.0, -9.8, 0.0);
        pbdConfig->m_dt         = 0.001;
        pbdConfig->m_iterations = 5;
        pbdConfig->m_linearDampingCoeff  = 0.02;
        pbdConfig->m_angularDampingCoeff = 0.02;
        pbdConfig->m_doPartitioning      = false;
        pbdModel->configure(pbdConfig);

        auto planeObj = std::make_shared<CollidingObject>("plane");

        auto plane = std::make_shared<Plane>(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
        plane->setWidth(1.0);
        auto planeGeom = plane;
    //auto planeGeom = GeometryUtils::toSurfaceMesh(plane);
    //auto  planeGeom = GeometryUtils::toTriangleGrid(Vec3d::Zero(), Vec2d(1, 1), Vec2i(4, 4));
        planeObj->setVisualGeometry(planeGeom);
        planeObj->setCollidingGeometry(planeGeom);
        m_scene->addSceneObject(planeObj);

        auto compoundRigid = makeCompoundObject(pbdModel, Vec3d(0.1, 0.15, 0.0));
        m_scene->addSceneObject(compoundRigid);

    {
        auto collision = std::make_shared<PbdObjectCollision>(compoundRigid, planeObj);
        collision->setUseCorrectVelocity(true);
        collision->setRigidBodyCompliance(0.0001);
        m_scene->addSceneObject(collision);
    }

        auto simpleRigid = makeTestSphereObject(pbdModel, Vec3d(-0.2, 0.15, 0.0));
        m_scene->addSceneObject(simpleRigid);
    {
        auto collision = std::make_shared<PbdObjectCollision>(simpleRigid, planeObj, "UnidirectionalPlaneToSphereCD");
        collision->setUseCorrectVelocity(true);
        collision->setRigidBodyCompliance(0.0001);
        m_scene->addSceneObject(collision);
        }

    // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
        m_scene->getConfig()->writeTaskGraph = true;

        std::shared_ptr<KeyboardDeviceClient> keyDevice = m_viewer->getKeyboardDevice();
        const Vec3d                           dx    = m_scene->getActiveCamera()->getPosition() - m_scene->getActiveCamera()->getFocalPoint();
        const double                          speed = 10.0;

        connect<Event>(m_sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            Vec3d extForce  = Vec3d(0.0, 0.0, 0.0);
            Vec3d extTorque = Vec3d(0.0, 0.0, 0.0);
            // If w down, move forward
            if (keyDevice->getButton('i') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, -1.0) * speed;
            }
            if (keyDevice->getButton('k') == KEY_PRESS)
            {
                extForce += Vec3d(0.0, 0.0, 1.0) * speed;
            }
            if (keyDevice->getButton('j') == KEY_PRESS)
            {
                extForce += Vec3d(-1.0, 0.0, 0.0) * speed;
            }
            if (keyDevice->getButton('l') == KEY_PRESS)
            {
                extForce += Vec3d(1.0, 0.0, 0.0) * speed;
            }
            // Apply torque around global y
            if (keyDevice->getButton('u') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, -0.1, 0.0);
            }
            if (keyDevice->getButton('o') == KEY_PRESS)
            {
                extTorque += Vec3d(0.0, 0.1, 0.0);
            }
            compoundRigid->getPbdBody()->externalForce  = extForce;
            compoundRigid->getPbdBody()->externalTorque = extTorque;
                });

        runFor(10.0);
}
