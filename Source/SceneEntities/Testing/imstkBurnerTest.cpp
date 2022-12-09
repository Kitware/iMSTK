/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBurner.h"
#include "imstkCapsule.h"
#include "imstkCollider.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkBurningTest, testState)
{
    // Setup the PBD Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->getConfig()->m_doPartitioning = false;
    pbdModel->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdModel->getConfig()->m_iterations = 4;
    pbdModel->getConfig()->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdModel->getConfig()->m_linearDampingCoeff  = 0.01; // Removed from velocity
    pbdModel->getConfig()->m_angularDampingCoeff = 0.01;

    // Create burning geometry
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.03);
    toolGeometry->setLength(0.4);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setOrientation(Quatd(0.707, 0.707, 0.0, 0.0));

    auto toolObj = std::make_shared<PbdObject>("Tool");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->addComponent<Collider>()->setGeometry(toolGeometry);
    toolObj->setDynamicalModel(pbdModel);
    toolObj->getPbdBody()->setRigid(
        Vec3d(0.0, 5.0, 2.0),
        1.0,
        Quatd::Identity(),
        Mat3d::Identity() * 1.0);

    // Create burning component
    auto burner = std::make_shared<Burner>();

    // Add component to geometry
    toolObj->addComponent(burner);

    burner->initialize();

    EXPECT_EQ(false, burner->getState());

    burner->start();
    EXPECT_EQ(true, burner->getState());
}