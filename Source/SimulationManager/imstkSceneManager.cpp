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

// imstk
#include "imstkScene.h"
#include "imstkSceneManager.h"
//#include "imstkCameraController.h"
#include "imstkCamera.h"

namespace imstk
{
SceneManager::SceneManager(std::shared_ptr<Scene> scene) :
    Module(scene->getName()),
    m_scene(scene)
{
}

void
SceneManager::initModule()
{
    m_scene->launchModules();
}

void
SceneManager::cleanUpModule()
{
    // End Camera Controller
    /// \todo move this out of scene manager
    /*if (auto camController = m_scene->getCamera()->getController())
    {
        camController->end();
        m_threadMap.at(camController->getName()).join();
    }*/
}

void
SceneManager::runModule()
{
    m_scene->advance();
};

void
SceneManager::startModuleInNewThread(std::shared_ptr<Module> module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}
} // imstk
