/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneManager.h"
#include "imstkScene.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Test we are able to set and switch active scene
///
TEST(imstkSceneManagerTest, TestSetActiveScene)
{
    auto sceneManager = std::make_shared<SceneManager>();
    auto scene1       = std::make_shared<Scene>("Scene1");
    sceneManager->setActiveScene(scene1);
    EXPECT_EQ(sceneManager->getActiveScene(), scene1);

    auto scene2 = std::make_shared<Scene>("Scene2");
    sceneManager->setActiveScene(scene2);
    EXPECT_EQ(sceneManager->getActiveScene(), scene2);
}