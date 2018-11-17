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
#include "imstkSceneManager.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectController.h"
#include "imstkDynamicObject.h"
#include "imstkPbdObject.h"
#include "imstkDeformableObject.h"
#include "imstkGeometryMap.h"
#include "imstkTimer.h"
#include "imstkPbdSolver.h"

#include "g3log/g3log.hpp"

namespace imstk
{
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
}

void
SceneManager::runModule()
{
    StopWatch wwt;
    wwt.start();

    // Reset Contact forces to 0
    for (auto obj : m_scene->getSceneObjects())
    {
        if (auto defObj = std::dynamic_pointer_cast<DeformableObject>(obj))
        {
            defObj->getContactForce().setConstant(0.0);
        }
        else if (auto collidingObj = std::dynamic_pointer_cast<CollidingObject>(obj))
        {
            collidingObj->resetForce();
        }        
    }

    // Update objects controlled by the device controllers
    for (auto controller : m_scene->getSceneObjectControllers())
    {
        controller->updateControlledObjects();
    }

    // Compute collision data per interaction pair
    for (auto intPair : m_scene->getCollisionGraph()->getInteractionPairList())
    {
        intPair->computeCollisionData();     
    }

    // Process collision data per interaction pair
    for (auto intPair : m_scene->getCollisionGraph()->getInteractionPairList())
    {    
        intPair->processCollisionData();
    }

    // Run the solvers
    for (auto solvers : m_scene->getSolvers())
    {
        solvers->solve();
    }

    // Apply updated forces on device
    for (auto controller : m_scene->getSceneObjectControllers())
    {
        controller->applyForces();
    }

    // Apply the geometry and apply maps to all the objects
    for (auto obj : m_scene->getSceneObjects())
    {
        obj->updateGeometries();
    }

    // Set the trackers of the scene object controllers to out-of-date
    for (auto controller : m_scene->getSceneObjectControllers())
    {
        controller->setTrackerToOutOfDate();
    }

    auto timeElapsed = wwt.getTimeElapsed(StopWatch::TimeUnitType::seconds);

    // Update time step size of the dynamic objects    
    for (auto obj : m_scene->getSceneObjects())
    {
        if (obj->getType() == SceneObject::Type::Pbd)
        {
            if (auto dynaObj = std::dynamic_pointer_cast<PbdObject>(obj))
            {
                if (dynaObj->getDynamicalModel()->getTimeStepSizeType() == TimeSteppingType::realTime)
                {
                    dynaObj->getDynamicalModel()->setTimeStep(timeElapsed);
                }
            }
        }
        else if (obj->getType() == SceneObject::Type::FEMDeformable)
        {
            if (auto dynaObj = std::dynamic_pointer_cast<DeformableObject>(obj))
            {
                if (dynaObj->getDynamicalModel()->getTimeStepSizeType() == TimeSteppingType::realTime)
                {
                    dynaObj->getDynamicalModel()->setTimeStep(timeElapsed);
                }
            }
        }
    }

    m_scene->setFPS(1./wwt.getTimeElapsed(StopWatch::TimeUnitType::seconds));
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
} // imstk
