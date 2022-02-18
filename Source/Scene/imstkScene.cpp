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
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkFeDeformableObject.h"
#include "imstkFemDeformableBodyModel.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"

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
    m_taskGraph(std::make_shared<TaskGraph>("Scene_" + name + "_Source", "Scene_" + name + "_Sink")),
    m_computeTimesLock(std::make_shared<ParallelUtils::SpinLock>())
{
    auto defaultCam = std::make_shared<Camera>();
    defaultCam->setPosition(0.0, 2.0, -15.0);
    defaultCam->setFocalPoint(0.0, 0.0, 0.0);

    auto debugCam = std::make_shared<Camera>();
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
    for (const auto& obj : m_sceneObjects)
    {
        CHECK(obj->initialize()) << "Error initializing scene object: " << obj->getName();
    }

    // Build the compute graph
    buildTaskGraph();

    // Opportunity for user configuration
    this->postEvent(Event(Scene::configureTaskGraph()));

    // Initialize the task graph
    initTaskGraph();

    // Init the debug camera to the bounding box of the visual geometries
    if (m_config->debugCamBoundingBox)
    {
        Vec3d globalMin, globalMax;
        Scene::computeBoundingBox(globalMin, globalMax);

        const Vec3d  center = (globalMin + globalMax) * 0.5;
        const double size   = (globalMax - globalMin).norm();
        m_cameras["debug"]->setFocalPoint(center);
        m_cameras["debug"]->setPosition(center + Vec3d(0.0, 1.0, 1.0).normalized() * size);
    }

    LOG(INFO) << "Scene '" << this->getName() << "' initialized!";
    return true;
}

void
Scene::computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent)
{
    if (this->getSceneObjects().size() == 0)
    {
        lowerCorner = Vec3d(0., 0., 0.);
        upperCorner = Vec3d(0., 0., 0.);
        return;
    }

    lowerCorner = Vec3d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
    upperCorner = Vec3d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);

    for (const auto& obj : m_sceneObjects)
    {
        for (const auto& visualModel : obj->getVisualModels())
        {
            Vec3d                     min  = Vec3d(IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX, IMSTK_DOUBLE_MAX);
            Vec3d                     max  = Vec3d(IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN, IMSTK_DOUBLE_MIN);
            std::shared_ptr<Geometry> geom = visualModel->getGeometry();
            if (geom != nullptr)
            {
                geom->computeBoundingBox(min, max);
                lowerCorner = lowerCorner.cwiseMin(min);
                upperCorner = upperCorner.cwiseMax(max);
            }
        }
    }
    const Vec3d range = upperCorner - lowerCorner;
    lowerCorner = lowerCorner - range * (paddingPercent / 100.0);
    upperCorner = upperCorner + range * (paddingPercent / 100.0);
}

void
Scene::buildTaskGraph()
{
    // Configures the edges/flow of the graph

    // Clear the compute graph of all nodes/edges except source+sink
    m_taskGraph->clear();
    m_taskGraph->addEdge(m_taskGraph->getSource(), m_taskGraph->getSink());

    // Setup all SceneObject compute graphs
    for (const auto& obj : m_sceneObjects)
    {
        obj->initGraphEdges();
    }

    // Nest all the SceneObject graphs within this Scene's ComputeGraph
    for (const auto& obj : m_sceneObjects)
    {
        std::shared_ptr<TaskGraph> objComputeGraph = obj->getTaskGraph();
        if (objComputeGraph != nullptr)
        {
            // Remove any unused nodes
            objComputeGraph = TaskGraph::removeUnusedNodes(objComputeGraph);
            // Add edges between any nodes that are marked critical and running simulatenously
            objComputeGraph = TaskGraph::resolveCriticalNodes(objComputeGraph);
            // Sum and nest the graph
            m_taskGraph->nestGraph(objComputeGraph, m_taskGraph->getSource(), m_taskGraph->getSink());
        }
    }

    // Remove any possible unused nodes
    m_taskGraph = TaskGraph::removeUnusedNodes(m_taskGraph);
    // Resolve criticals across objects
    m_taskGraph = TaskGraph::resolveCriticalNodes(m_taskGraph);
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

    if (TaskGraph::isCyclic(m_taskGraph))
    {
        if (m_config->writeTaskGraph)
        {
            TaskGraphVizWriter writer;
            writer.setInput(m_taskGraph);
            writer.setFileName("sceneTaskGraph.svg");
            writer.write();
        }
        LOG(FATAL) << "Scene TaskGraph is cyclic, cannot proceed";
        return;
    }
    // Clean up graph if user wants
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

void
Scene::setEnableTaskTiming(const bool enabled)
{
    m_config->taskTimingEnabled = enabled;
    // If user wants to benchmark, tell all the nodes to time themselves
    for (std::shared_ptr<TaskNode> node : m_taskGraph->getNodes())
    {
        node->m_enableTiming = m_config->taskTimingEnabled;
    }
}

void
Scene::addInteraction(std::shared_ptr<SceneObject> interaction)
{
    addSceneObject(interaction);
}

std::shared_ptr<SceneObject>
Scene::getSceneObject(const std::string& name) const
{
    auto iter = std::find_if(m_sceneObjects.begin(), m_sceneObjects.end(),
        [name](const std::shared_ptr<SceneObject>& i) { return i->getName() == name; });
    return (iter == m_sceneObjects.end()) ? nullptr : *iter;
}

void
Scene::addSceneObject(std::shared_ptr<SceneObject> newSceneObject)
{
    std::string name = newSceneObject->getName();

    if (this->getSceneObject(name) != nullptr)
    {
        LOG(WARNING) << "Can not add object: '" << name
                     << "' is already registered in this scene.";
        return;
    }

    m_sceneObjects.insert(newSceneObject);
    this->postEvent(Event(modified()));
    LOG(INFO) << name << " object added to " << m_name;
}

void
Scene::removeSceneObject(const std::string& name)
{
    std::shared_ptr<SceneObject> obj = getSceneObject(name);
    if (obj == nullptr)
    {
        LOG(WARNING) << "No object named '" << name
                     << "' was registered in this scene.";
        return;
    }
    removeSceneObject(obj);
}

void
Scene::removeSceneObject(std::shared_ptr<SceneObject> sceneObject)
{
    if (m_sceneObjects.count(sceneObject) != 0)
    {
        m_sceneObjects.erase(sceneObject);
        this->postEvent(Event(modified()));
        LOG(INFO) << sceneObject->getName() << " object removed from scene " << m_name;
    }
    else
    {
        LOG(WARNING) << "Could not remove SceneObject '" << sceneObject->getName() << "', does not exist in the scene";
        return;
    }
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
    if ((m_lightsMap.find(lightName) == m_lightsMap.end()))
    {
        LOG(WARNING) << "No light named '" << lightName
                     << "' was registered in this scene.";
        return nullptr;
    }

    return m_lightsMap.at(lightName);
}

void
Scene::addLight(const std::string& name, std::shared_ptr<Light> newLight)
{
    if (m_lightsMap.find(name) != m_lightsMap.cend())
    {
        LOG(WARNING) << "Can not add light: '" << name
                     << "' is already registered in this scene.";
        return;
    }

    m_lightsMap[name] = newLight;
    this->postEvent(Event(modified()));
    LOG(INFO) << name << " light added to " << m_name;
}

void
Scene::removeLight(const std::string& lightName)
{
    if (this->getLight(lightName) == nullptr)
    {
        LOG(WARNING) << "No light named '" << lightName
                     << "' was registered in this scene.";
        return;
    }

    m_lightsMap.erase(lightName);
    LOG(INFO) << lightName << " light removed from " << m_name;
}

std::string
Scene::getCameraName(const std::shared_ptr<Camera> cam) const
{
    using MapType = std::unordered_map<std::string, std::shared_ptr<Camera>>;
    auto i = std::find_if(m_cameras.begin(), m_cameras.end(),
        [&cam](const MapType::value_type& j) { return j.second == cam; });
    if (i != m_cameras.end())
    {
        return i->first;
    }
    else
    {
        return "";
    }
}

std::shared_ptr<imstk::Camera>
Scene::getCamera(const std::string name) const
{
    auto i = m_cameras.find(name);
    if (i != m_cameras.end())
    {
        return i->second;
    }
    else
    {
        return nullptr;
    }
}

void
Scene::addCamera(const std::string& name, std::shared_ptr<Camera> cam)
{
    if (m_cameras.find(name) != m_cameras.end())
    {
        LOG(WARNING) << "Cannot add camera: Camera with the name " << name << " already exists.";
    }
    m_cameras[name] = cam;
}

void
Scene::setActiveCamera(const std::string name)
{
    auto i = m_cameras.find(name);
    if (i != m_cameras.end())
    {
        m_activeCamera = m_cameras[name];
    }
}

void
Scene::removeCamera(const std::string name)
{
    auto i = m_cameras.find(name);
    if (i != m_cameras.end() && !(name == "default" || name == "debug"))
    {
        m_cameras.erase(name);
        LOG(INFO) << name << " camera removed from " << m_name;
    }
    else
    {
        LOG(WARNING) << "No camera named '" << name
                     << "' is part of the scene.";
    }
}

void
Scene::addController(std::shared_ptr<TrackingDeviceControl> controller)
{
    m_trackingControllers.push_back(controller);
    this->postEvent(Event(modified()));
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
    for (auto obj : m_sceneObjects)
    {
        obj->reset();
    }
}

void
Scene::advance(const double dt)
{
    StopWatch wwt;
    wwt.start();

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

    // Reset Contact forces to 0
    for (auto obj : this->getSceneObjects())
    {
        if (auto defObj = std::dynamic_pointer_cast<FeDeformableObject>(obj))
        {
            defObj->getFEMModel()->getContactForce().setConstant(0.0);
        }
    }

    // Update objects controlled by the device controllers
    for (auto controller : this->getControllers())
    {
        controller->update(dt);
    }

    // Execute the computational graph
    if (m_taskGraphController != nullptr)
    {
        m_taskGraphController->execute();
    }

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

    if (m_resetRequested)
    {
        resetSceneObjects();
        //\note May need to reset CD, CH and other components of the scene in the future
        m_resetRequested = false;
    }

    // FPS of physics is given by the measured time, not the given time step dt
    const double elapsedTime = wwt.getTimeElapsed(StopWatch::TimeUnitType::seconds);
    m_fps = 1.0 / elapsedTime;

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

void
Scene::updateVisuals()
{
    for (auto obj : m_sceneObjects)
    {
        obj->visualUpdate();
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
} // namespace imstk