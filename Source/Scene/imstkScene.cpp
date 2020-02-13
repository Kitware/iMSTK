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

#include "imstkScene.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectControllerBase.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkPbdObject.h"
#include "imstkDeformableObject.h"
#include "imstkTimer.h"
#include "imstkPbdSolver.h"
#include "imstkRigidBodyWorld.h"

#include <g3log/g3log.hpp>

namespace imstk
{
Scene::~Scene()
{
    // End Camera Controller
    if (auto camController = this->getCamera()->getController())
    {
        camController->end();
        m_threadMap.at(camController->getName()).join();
    }
}

bool
Scene::initialize()
{
    for (auto const& it : m_sceneObjectsMap)
    {
        auto sceneObject = it.second;
        if (!sceneObject->initialize())
        {
            LOG(FATAL) << "Error initializing scene object: " << sceneObject->getName();
            return false;
        }
    }
    m_isInitialized = true;
    return true;
}

void
Scene::launchModules()
{
    // Start Camera Controller (asynchronous)
    if (auto camController = this->getCamera()->getController())
    {
        m_threadMap[camController->getName()] = std::thread([camController] { camController->start(); });
    }
}

bool
Scene::isObjectRegistered(const std::string& sceneObjectName) const
{
    return m_sceneObjectsMap.find(sceneObjectName) != m_sceneObjectsMap.end();
}

const std::vector<std::shared_ptr<SceneObject>>
Scene::getSceneObjects() const
{
    std::vector<std::shared_ptr<SceneObject>> v;

    for (auto it = m_sceneObjectsMap.begin();
         it != m_sceneObjectsMap.end();
         ++it)
    {
        v.push_back(it->second);
    }

    return v;
}

const std::vector<std::shared_ptr<SceneObjectControllerBase>>
Scene::getSceneObjectControllers() const
{
    return m_objectControllers;
}

std::shared_ptr<SceneObject>
Scene::getSceneObject(const std::string& sceneObjectName) const
{
    if (!this->isObjectRegistered(sceneObjectName))
    {
        LOG(FATAL) << "No scene object named '" << sceneObjectName
                   << "' was registered in this scene.";
        return nullptr;
    }

    return m_sceneObjectsMap.at(sceneObjectName);
}

const std::vector<std::shared_ptr<DebugRenderGeometry>>
Scene::getDebugRenderObjects() const
{
    std::vector<std::shared_ptr<DebugRenderGeometry>> v;

    for (auto it = m_DebugRenderGeometryMap.begin();
         it != m_DebugRenderGeometryMap.end();
         ++it)
    {
        v.push_back(it->second);
    }

    return v;
}

void
Scene::addSceneObject(std::shared_ptr<SceneObject> newSceneObject)
{
    std::string newSceneObjectName = newSceneObject->getName();

    if (this->isObjectRegistered(newSceneObjectName))
    {
        LOG(WARNING) << "Can not add object: '" << newSceneObjectName
                     << "' is already registered in this scene.";
        return;
    }

    m_sceneObjectsMap[newSceneObjectName] = newSceneObject;
    LOG(INFO) << newSceneObjectName << " object added to " << m_name;
}

void
Scene::addDebugGeometry(std::shared_ptr<DebugRenderGeometry> newDebugRenderObject)
{
    std::string name = newDebugRenderObject->getName();

    if (m_sceneObjectsMap.find(name) != m_sceneObjectsMap.end())
    {
        LOG(WARNING) << "Can not add debug render object: '" << name
                     << "' is already registered in this scene.";
        return;
    }

    m_DebugRenderGeometryMap[name] = newDebugRenderObject;
    LOG(INFO) << name << " object added to " << m_name;
}

void
Scene::removeSceneObject(const std::string& sceneObjectName)
{
    if (!this->isObjectRegistered(sceneObjectName))
    {
        LOG(WARNING) << "No object named '" << sceneObjectName
                     << "' was registered in this scene.";
        return;
    }

    m_sceneObjectsMap.erase(sceneObjectName);
    LOG(INFO) << sceneObjectName << " object removed from " << m_name;
}

bool
Scene::isLightRegistered(const std::string& lightName) const
{
    return m_lightsMap.find(lightName) != m_lightsMap.end();
}

const std::vector<std::shared_ptr<Light>>
Scene::getLights() const
{
    std::vector<std::shared_ptr<Light>> v;

    for (auto it = m_lightsMap.begin();
         it != m_lightsMap.end();
         ++it)
    {
        v.push_back(it->second);
    }

    return v;
}

std::shared_ptr<Light>
Scene::getLight(const std::string& lightName) const
{
    if (!this->isLightRegistered(lightName))
    {
        LOG(WARNING) << "No light named '" << lightName
                     << "' was registered in this scene.";
        return nullptr;
    }

    return m_lightsMap.at(lightName);
}

void
Scene::addLight(std::shared_ptr<Light> newLight)
{
    std::string newlightName = newLight->getName();

    if (this->isLightRegistered(newlightName))
    {
        LOG(WARNING) << "Can not add light: '" << newlightName
                     << "' is already registered in this scene.";
        return;
    }

    m_lightsMap[newlightName] = newLight;
    LOG(INFO) << newlightName << " light added to " << m_name;
}

void
Scene::removeLight(const std::string& lightName)
{
    if (!this->isLightRegistered(lightName))
    {
        LOG(WARNING) << "No light named '" << lightName
                     << "' was registered in this scene.";
        return;
    }

    m_lightsMap.erase(lightName);
    LOG(INFO) << lightName << " light removed from " << m_name;
}

void
Scene::setGlobalIBLProbe(std::shared_ptr<IBLProbe> newIBLProbe)
{
    m_globalIBLProbe = newIBLProbe;
}

std::shared_ptr<IBLProbe>
Scene::getGlobalIBLProbe()
{
    return m_globalIBLProbe;
}

const std::string&
Scene::getName() const
{
    return m_name;
}

std::shared_ptr<Camera>
Scene::getCamera() const
{
    return m_camera;
}

std::shared_ptr<CollisionGraph>
Scene::getCollisionGraph() const
{
    return m_collisionGraph;
}

const std::vector<std::shared_ptr<SolverBase>>
Scene::getSolvers()
{
    return m_solvers;
}

void
Scene::addNonlinearSolver(std::shared_ptr<SolverBase> solver)
{
    m_solvers.push_back(solver);
}

void
Scene::addObjectController(std::shared_ptr<SceneObjectControllerBase> controller)
{
    m_objectControllers.push_back(controller);
}

void
Scene::reset()
{
    // Apply the geometry and apply maps to all the objects
    for (auto obj : this->getSceneObjects())
    {
        const auto objType = obj->getType();
        if (objType == SceneObject::Type::Rigid
            || objType == SceneObject::Type::FEMDeformable
            || objType == SceneObject::Type::Pbd
            || objType == SceneObject::Type::SPH)
        {
            obj->reset();
        }
    }
}

void
Scene::advance(double dt)
{
    // Update time step size of the dynamic objects
    for (auto obj : this->getSceneObjects())
    {
        if (obj->getType() == SceneObject::Type::Pbd)
        {
            if (auto dynaObj = std::dynamic_pointer_cast<PbdObject>(obj))
            {
                if (dynaObj->getDynamicalModel()->getTimeStepSizeType() == TimeSteppingType::realTime)
                {
                    dynaObj->getDynamicalModel()->setTimeStep(dt);
                }
            }
        }
        else if (obj->getType() == SceneObject::Type::FEMDeformable)
        {
            if (auto dynaObj = std::dynamic_pointer_cast<DeformableObject>(obj))
            {
                if (dynaObj->getDynamicalModel()->getTimeStepSizeType() == TimeSteppingType::realTime)
                {
                    dynaObj->getDynamicalModel()->setTimeStep(dt);
                }
            }
        }
    }

    // PhysX update; move this to solver
    auto physxScene = RigidBodyWorld::getInstance()->m_Scene;
    physxScene->simulate(1.0f / 300.0f); // TODO: update the time step
    physxScene->fetchResults(true);

    // Reset Contact forces to 0
    for (auto obj : this->getSceneObjects())
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
    for (auto controller : this->getSceneObjectControllers())
    {
        controller->updateControlledObjects();
    }

    // Update the static octree and perform collision detection for some collision pairs
    CollisionDetection::updateInternalOctreeAndDetectCollision();

    // Compute collision data per interaction pair
    for (auto intPair : this->getCollisionGraph()->getInteractionPairList())
    {
        intPair->computeCollisionData();
    }

    // Process collision data per interaction pair
    for (auto intPair : this->getCollisionGraph()->getInteractionPairList())
    {
        intPair->processCollisionData();
    }

    // Run the solvers
    for (auto solvers : this->getSolvers())
    {
        solvers->solve();
    }

    // Apply updated forces on device
    for (auto controller : this->getSceneObjectControllers())
    {
        controller->applyForces();
    }

    // Apply the geometry and apply maps to all the objects
    for (auto obj : this->getSceneObjects())
    {
        obj->updateGeometries();
    }

    // Set the trackers of the scene object controllers to out-of-date
    for (auto controller : this->getSceneObjectControllers())
    {
        controller->setTrackerToOutOfDate();
    }
}
} // imstk
