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
#include "imstkVirtualCouplingObject.h"
#include "imstkDynamicObject.h"

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
    // Start Camera Controller (asynchronous)
    if (auto camController = m_scene->getCamera()->getController())
    {
        this->startModuleInNewThread(camController);
    }

    // Init virtual coupling objects offsets
    for (auto obj : m_scene->getSceneObjects())
    {
        if (auto virtualCoupling = std::dynamic_pointer_cast<VirtualCouplingObject>(obj))
        {
            virtualCoupling->initOffsets();
        }
    }
}

void
SceneManager::runModule()
{
    // Compute collision data per interaction pair
    for (auto intPair : m_scene->getCollisionGraph()->getInteractionPairList())
    {
        intPair->computeCollisionData();
        intPair->computeContactForces();
    }

    // Update collision handlers

    // Update the nonlinear solvers
    for (auto nlSolvers : m_scene->getNonlinearSolvers())
    {
        nlSolvers->solveSimple();
    }

    // Apply the geometry maps
    for (auto obj : m_scene->getSceneObjects())
    {
        if (auto dynaObj = std::dynamic_pointer_cast<DynamicObject>(obj))
        {
            dynaObj->getDynamicalModel()->updatePhysicsGeometry();
            //dynaObj->getPhysicsToCollidingMap()->apply();
            dynaObj->getPhysicsToVisualMap()->apply();
        }
    }

    // Update virtualCoupling objects based on devices
    for (auto obj : m_scene->getSceneObjects())
    {
        if (auto virtualCoupling = std::dynamic_pointer_cast<VirtualCouplingObject>(obj))
        {
            virtualCoupling->updateFromDevice();
            virtualCoupling->applyForces();
        }
    }
}

void
SceneManager::cleanUpModule()
{
    // End Camera Controller
    if (auto camController = m_scene->getCamera()->getController())
    {
        camController->end();
        m_threadMap.at(camController->getName()).join();
    }
}
void
SceneManager::startModuleInNewThread(std::shared_ptr<Module> module)
{
    m_threadMap[module->getName()] = std::thread([module] { module->start(); });
}

}
