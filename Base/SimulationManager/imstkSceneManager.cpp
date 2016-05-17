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

#include "imstkCameraController.h"

#include "g3log/g3log.hpp"

namespace imstk {

std::shared_ptr<Scene>
SceneManager::getScene()
{
    return m_scene;
}

void
SceneManager::initModule()
{
    LOG(DEBUG) << m_name << " manager : init";


    if (auto camController = m_scene->getCamera()->getController())
    {
        this->startModuleInNewThread(camController);
    }
}

void
SceneManager::cleanUpModule()
{
    LOG(DEBUG) << m_name << " manager : cleanUp";

    if (auto camController = m_scene->getCamera()->getController())
    {
        camController->end();
        m_threadMap.at(camController->getName()).join();
    }
}

void
SceneManager::runModule()
{
    LOG(DEBUG) << m_name << " manager : running";
}

void
SceneManager::startModuleInNewThread(std::shared_ptr<Module> module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}
}
