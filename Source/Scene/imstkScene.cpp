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
#include "imstkCamera.h"
#include "imstkCameraController.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionGraph.h"
#include "imstkCollisionPair.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkFeDeformableObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkRigidBodyWorld.h"
#include "imstkSequentialTaskGraphController.h"
#include "imstkTaskGraph.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkTbbTaskGraphController.h"
#include "imstkTimer.h"
#include "imstkTrackingDeviceControl.h"
#include "imstkVisualModel.h"

namespace imstk
{
Scene::Scene(const std::string& name, std::shared_ptr<SceneConfig> config) :
    m_config(config),
    m_name(name),
    m_activeCamera(nullptr),
    m_collisionGraph(std::make_shared<CollisionGraph>()),
    m_taskGraph(std::make_shared<TaskGraph>("Scene_" + name + "_Source", "Scene_" + name + "_Sink")),
    m_computeTimesLock(std::make_shared<ParallelUtils::SpinLock>())
{
    std::shared_ptr<Camera> defaultCam = std::make_shared<Camera>();
    defaultCam->setPosition(0.0, 2.0, -15.0);
    defaultCam->setFocalPoint(0.0, 0.0, 0.0);

    std::shared_ptr<Camera> debugCam = std::make_shared<Camera>();
    debugCam->setPosition(0.0, 4.0, -30.0);
    debugCam->setFocalPoint(0.0, 0.0, 0.0);

    m_cameras["default"] = defaultCam;
    m_cameras["debug"]   = debugCam;
    setActiveCamera("default");
}

bool
Scene::initialize()
{
    // Initialize all the SceneObjects
    for (auto const& it : m_sceneObjectsMap)
    {
        auto sceneObject = it.second;
        CHECK(sceneObject->initialize()) << "Error initializing scene object: " << sceneObject->getName();
    }

    // Build the compute graph
    buildTaskGraph();

    // Opportunity for user configuration
    this->postEvent(Event(EventType::Configure));

    // Then init
    initTaskGraph();

    // Init the debug camera to the bounding box of the visual geometries
    if (m_config->debugCamBoundingBox)
    {
        Vec3d globalMin = Vec3d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
        Vec3d globalMax = Vec3d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
        for (auto i : m_sceneObjectsMap)
        {
            std::shared_ptr<SceneObject> obj = i.second;
            for (auto visualModels : obj->getVisualModels())
            {
                Vec3d min = Vec3d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
                Vec3d max = Vec3d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
                visualModels->getGeometry()->computeBoundingBox(min, max);
                globalMin = globalMin.cwiseMin(min);
                globalMax = globalMax.cwiseMax(max);
            }
        }
        const Vec3d center = (globalMin + globalMax) * 0.5;
        const Vec3d size   = globalMax - globalMin;
        m_cameras["debug"]->setFocalPoint(center);
        m_cameras["debug"]->setPosition(center + Vec3d(0.0, 1.0, 1.0).normalized() * size.norm());
    }

    LOG(INFO) << "Scene '" << this->getName() << "' initialized!";
    return true;
}

void
Scene::buildTaskGraph()
{
    // Configures the edges/flow of the graph

    // Clear the compute graph of all nodes/edges except source+sink
    m_taskGraph->clear();
    m_taskGraph->addEdge(m_taskGraph->getSource(), m_taskGraph->getSink());

    // Setup all SceneObject compute graphs (and segment the rigid bodies)
    std::list<std::shared_ptr<SceneObject>> rigidBodies;
    for (auto const& it : m_sceneObjectsMap)
    {
        auto sceneObject = it.second;
        if (sceneObject->getTypeName() == "RigidObject")
        {
            rigidBodies.push_back(sceneObject);
        }

        sceneObject->initGraphEdges();
    }

    // Apply all the interaction graph element operations to the SceneObject graphs
    const std::vector<std::shared_ptr<ObjectInteractionPair>>& pairs = m_collisionGraph->getInteractionPairs();
    for (size_t i = 0; i < pairs.size(); i++)
    {
        pairs[i]->apply();
    }

    // Nest all the SceneObject graphs within this Scene's ComputeGraph
    for (auto const& it : m_sceneObjectsMap)
    {
        std::shared_ptr<TaskGraph> objComputeGraph = it.second->getTaskGraph();
        if (objComputeGraph != nullptr)
        {
            // Add edges between any nodes that are marked critical and running simulatenously
            objComputeGraph = TaskGraph::resolveCriticalNodes(objComputeGraph);
            // Sum and nest the graph
            m_taskGraph->nestGraph(objComputeGraph, m_taskGraph->getSource(), m_taskGraph->getSink());
        }
    }

    // Edge Case: Rigid bodies all have a singular update point because of how PhysX works
    // Think about generalizes these islands of interaction to Systems
    if (rigidBodies.size() > 0)
    {
        // The node that updates the rigid body system
        auto physXUpdate = m_taskGraph->addFunction("PhysXUpdate", [&]()
            {
                auto physxScene = RigidBodyWorld::getInstance()->m_Scene;
                // \todo: update the time step, split into two steps, collide and advance
                physxScene->simulate(RigidBodyWorld::getInstance()->getTimeStep());
                physxScene->fetchResults(true);
            });

        // Scene Source->physX Update->Rigid Body Update Geometry[i]
        m_taskGraph->addEdge(m_taskGraph->getSource(), physXUpdate);
        m_taskGraph->addEdge(physXUpdate, m_taskGraph->getSink());
        for (std::list<std::shared_ptr<SceneObject>>::iterator i = rigidBodies.begin(); i != rigidBodies.end(); i++)
        {
            m_taskGraph->addEdge(physXUpdate, (*i)->getUpdateGeometryNode());
        }
    }
}

void
Scene::initTaskGraph()
{
    // Pick a controller for the graph execution
    if (m_config->taskParallelizationEnabled)
    {
        m_taskGraphController = std::make_shared<TbbTaskGraphController>();
    }
    else
    {
        m_taskGraphController = std::make_shared<SequentialTaskGraphController>();
    }

    // Reduce the graph, removing nonfunctional nodes, and redundant edges
    if (m_config->graphReductionEnabled)
    {
        m_taskGraph = TaskGraph::reduce(m_taskGraph);
    }

    // If user wants to benchmark, tell all the nodes to time themselves
    for (std::shared_ptr<TaskNode> node : m_taskGraph->getNodes())
    {
        node->m_enableTiming = m_config->taskTimingEnabled;
    }

    // Generate unique names among the nodes
    TaskGraph::getUniqueNodeNames(m_taskGraph, true);
    m_nodeComputeTimes.clear();

    if (m_config->writeTaskGraph)
    {
        TaskGraphVizWriter writer;
        writer.setInput(m_taskGraph);
        writer.setFileName("sceneTaskGraph.svg");
        writer.write();
    }

    m_taskGraphController->setTaskGraph(m_taskGraph);
    m_taskGraphController->initialize();
}

bool
Scene::isObjectRegistered(const std::string& sceneObjectName) const
{
    return m_sceneObjectsMap.find(sceneObjectName) != m_sceneObjectsMap.end();
}

void
Scene::setEnableTaskTiming(bool enabled)
{
    m_config->taskTimingEnabled = enabled;
    // If user wants to benchmark, tell all the nodes to time themselves
    for (std::shared_ptr<TaskNode> node : m_taskGraph->getNodes())
    {
        node->m_enableTiming = m_config->taskTimingEnabled;
    }
}

const std::vector<std::shared_ptr<SceneObject>>
Scene::getSceneObjects() const
{
    std::vector<std::shared_ptr<SceneObject>> v;

    for (auto it : m_sceneObjectsMap)
    {
        v.push_back(it.second);
    }

    return v;
}

std::shared_ptr<SceneObject>
Scene::getSceneObject(const std::string& sceneObjectName) const
{
    CHECK(this->isObjectRegistered(sceneObjectName))
        << "No scene object named '" << sceneObjectName
        << "' was registered in this scene.";

    return m_sceneObjectsMap.at(sceneObjectName);
}

const std::vector<std::shared_ptr<VisualModel>>
Scene::getDebugRenderModels() const
{
    std::vector<std::shared_ptr<VisualModel>> v;

    for (auto it : m_DebugRenderModelMap)
    {
        v.push_back(it.second);
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
Scene::addDebugVisualModel(std::shared_ptr<VisualModel> dbgRenderModel)
{
    const std::string name = dbgRenderModel->getDebugGeometry()->getName();

    if (m_DebugRenderModelMap.find(name) != m_DebugRenderModelMap.end())
    {
        LOG(WARNING) << "Can not add debug render mdoel: '" << name
                     << "' is already registered in this scene.";
        return;
    }

    m_DebugRenderModelMap[name] = dbgRenderModel;
    LOG(INFO) << name << " debug model added to " << m_name;
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
Scene::addController(std::shared_ptr<TrackingDeviceControl> controller)
{
    m_trackingControllers.push_back(controller);
}

void
Scene::reset()
{
    m_resetRequested = true;
}

void
Scene::resetSceneObjects()
{
    // Apply the geometry and apply maps to all the objects
    for (auto obj : this->getSceneObjects())
    {
        obj->reset();
    }

    //\todo reset the timestep to the fixed default value when paused->run or reset
}

void
Scene::advance()
{
    StopWatch wwt;
    wwt.start();

    advance(m_elapsedTime);

    m_elapsedTime = wwt.getTimeElapsed(StopWatch::TimeUnitType::seconds);
}

void
Scene::advance(const double dt)
{
    // Reset Contact forces to 0
    for (auto obj : this->getSceneObjects())
    {
        if (auto defObj = std::dynamic_pointer_cast<FeDeformableObject>(obj))
        {
            defObj->getFEMModel()->getContactForce().setConstant(0.0);
        }
        else if (auto collidingObj = std::dynamic_pointer_cast<CollidingObject>(obj))
        {
            collidingObj->resetForce();
        }
    }

    // Update objects controlled by the device controllers
    for (auto controller : this->getControllers())
    {
        controller->update(dt);
    }

    CollisionDetection::updateInternalOctreeAndDetectCollision();

    // Execute the computational graph
    m_taskGraphController->execute();

    // Apply updated forces on device
    for (auto controller : this->getControllers())
    {
        controller->applyForces();
    }

    // Set the trackers of the scene object controllers to out-of-date
    for (auto controller : this->getControllers())
    {
        controller->setTrackerToOutOfDate();
    }

    for (auto obj : this->getSceneObjects())
    {
        if (auto dynaObj = std::dynamic_pointer_cast<DynamicObject>(obj))
        {
            if (dynaObj->getDynamicalModel()->getTimeStepSizeType() == TimeSteppingType::RealTime)
            {
                dynaObj->getDynamicalModel()->setTimeStep(dt);
            }
        }
    }

    if (m_resetRequested)
    {
        resetSceneObjects();
        //\note May need to reset CD, CH and other components of the scene in the future
        m_resetRequested = false;
    }

    this->setFPS(1.0 / dt);

    // If benchmarking enabled, produce a time table for each step
    if (m_config->taskTimingEnabled)
    {
        lockComputeTimes();
        for (std::shared_ptr<TaskNode> node : m_taskGraph->getNodes())
        {
            m_nodeComputeTimes[node->m_name] = node->m_computeTime;
        }
        unlockComputeTimes();
    }
}

double
Scene::getElapsedTime(const std::string& stepName) const
{
    if (m_nodeComputeTimes.count(stepName) == 0)
    {
        LOG(WARNING) << "Tried to get elapsed time of nonexistent step. Is benchmarking enabled?";
        return 0.0;
    }
    else
    {
        return m_nodeComputeTimes.at(stepName);
    }
}

void
Scene::lockComputeTimes()
{
    m_computeTimesLock->lock();
}

void
Scene::unlockComputeTimes()
{
    m_computeTimesLock->unlock();
}
} // imstk
