/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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