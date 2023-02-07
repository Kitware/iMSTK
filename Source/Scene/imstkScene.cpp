/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkScene.h"
#include "imstkCamera.h"
#include "imstkDeviceControl.h"
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
#include "imstkAbstractDynamicalModel.h"

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
    // Gather all the systems from the object components
    // Right now this just includes DynamicalModel's
    std::unordered_set<std::shared_ptr<AbstractDynamicalModel>> systems;
    for (const auto& ent : m_sceneEntities)
    {
        if (auto dynObj = std::dynamic_pointer_cast<DynamicObject>(ent))
        {
            systems.insert(dynObj->getDynamicalModel());
        }
    }

    // Initialize all the SceneObjects
    for (const auto& ent : m_sceneEntities)
    {
        if (auto obj = std::dynamic_pointer_cast<SceneObject>(ent))
        {
            CHECK(obj->initialize()) << "Error initializing scene object: " << obj->getName();

            // Print any controls
            if (auto deviceObj = std::dynamic_pointer_cast<DeviceControl>(obj))
            {
                deviceObj->printControls();
            }
        }
    }

    // Initialize all components
    // If any components are added during initialize, initialize those
    // as well, do this until all are initialized
    std::unordered_map<std::shared_ptr<Component>, bool> compIsInitd;
    for (const auto& ent : m_sceneEntities)
    {
        std::vector<std::shared_ptr<Component>> compsToInit = ent->getComponents();
        while (compsToInit.size() > 0)
        {
            // Initialize all components and denote which are now complete
            for (const auto& comp : compsToInit)
            {
                comp->initialize();
                compIsInitd[comp] = true;
            }

            // Rnu through all the components again, if any were added (found not init'd)
            // then add them to the compsToInit
            std::vector<std::shared_ptr<Component>> newComps = ent->getComponents();
            compsToInit.clear();
            for (const auto& comp : newComps)
            {
                if (!compIsInitd[comp])
                {
                    compsToInit.push_back(comp);
                }
            }
        }
    }

    // Initialize all systems
    for (const auto& system : systems)
    {
        CHECK(system->initialize()) << "Error initializing system";
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

    m_sceneTime = 0.0;

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

    for (const auto& ent : m_sceneEntities)
    {
        for (const auto& visualModel : ent->getComponents<VisualModel>())
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

    // Setup all SceneObject & their components compute graphs
    // \todo: Remove SceneObject TaskGraphs
    for (const auto& ent : m_sceneEntities)
    {
        if (auto obj = std::dynamic_pointer_cast<SceneObject>(ent))
        {
            obj->initGraphEdges();
        }
        for (const auto& comp : ent->getComponents())
        {
            auto behaviour = std::dynamic_pointer_cast<SceneBehaviour>(comp);
            if (behaviour != nullptr && behaviour->getTaskGraph() != nullptr)
            {
                behaviour->initTaskGraphEdges();
            }
        }
    }

    // Nest all the SceneObject graphs & TaskBehaviour graphs within this Scene's ComputeGraph
    // \todo: Remove SceneObject TaskGraphs
    for (const auto& ent : m_sceneEntities)
    {
        if (auto obj = std::dynamic_pointer_cast<SceneObject>(ent))
        {
            std::shared_ptr<TaskGraph> taskGraph = obj->getTaskGraph();
            if (taskGraph != nullptr)
            {
                // Remove any unused nodes
                taskGraph = TaskGraph::removeUnusedNodes(taskGraph);
                // Sum and nest the graph
                m_taskGraph->nestGraph(taskGraph, m_taskGraph->getSource(), m_taskGraph->getSink());
            }
        }
        for (const auto& comp : ent->getComponents())
        {
            if (auto taskBehaviour = std::dynamic_pointer_cast<SceneBehaviour>(comp))
            {
                std::shared_ptr<TaskGraph> taskGraph = taskBehaviour->getTaskGraph();
                if (taskGraph != nullptr)
                {
                    // Remove any unused nodes
                    taskGraph = TaskGraph::removeUnusedNodes(taskGraph);
                    // Sum and nest the graph
                    m_taskGraph->nestGraph(taskGraph, m_taskGraph->getSource(), m_taskGraph->getSink());
                }
            }
        }
    }

    // Remove any possible unused nodes
    m_taskGraph = TaskGraph::removeUnusedNodes(m_taskGraph);
}

void
Scene::initTaskGraph()
{
    m_taskGraphController = std::make_shared<SequentialTaskGraphController>();

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
Scene::addInteraction(std::shared_ptr<Entity> interaction)
{
    addSceneObject(interaction);
}

std::shared_ptr<Entity>
Scene::getSceneObject(const std::string& name) const
{
    auto iter = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(),
        [name](const std::shared_ptr<Entity>& i) { return i->getName() == name; });
    return (iter == m_sceneEntities.end()) ? nullptr : *iter;
}

void
Scene::addSceneObject(std::shared_ptr<Entity> entity)
{
    // If already exists, exit
    if (m_sceneEntities.find(entity) != m_sceneEntities.end())
    {
        LOG(WARNING) << "Entity " << entity->getName() << " already in the scene, not added";
        return;
    }

    // Ensure the name is unique
    const std::string orgName    = entity->getName();
    const std::string uniqueName = getUniqueName(orgName);
    if (orgName != uniqueName)
    {
        LOG(INFO) << "Entity with name " << orgName << " already in scene. Renamed to " << uniqueName;
        entity->setName(uniqueName);
    }

    m_sceneEntities.insert(entity);
    this->postEvent(Event(modified()));
    LOG(INFO) << uniqueName << " entity added to " << m_name << " scene";
}

void
Scene::removeSceneObject(const std::string& name)
{
    std::shared_ptr<Entity> ent = getSceneObject(name);
    if (ent == nullptr)
    {
        LOG(WARNING) << "No entity named '" << name
                     << "' was registered in this scene.";
        return;
    }
    removeSceneObject(ent);
}

void
Scene::removeSceneObject(std::shared_ptr<Entity> entity)
{
    if (m_sceneEntities.count(entity) != 0)
    {
        m_sceneEntities.erase(entity);
        this->postEvent(Event(modified()));
        LOG(INFO) << entity->getName() << " object removed from scene " << m_name;
    }
    else
    {
        LOG(WARNING) << "Could not remove Entity '" << entity->getName() << "', does not exist in the scene";
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
        LOG(WARNING) << "Cannot add light: '" << name
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
Scene::getUniqueName(const std::string& name) const
{
    int         i = 1;
    std::string uniqueName = name;
    // While name is not unique, iterate it
    while (getSceneObject(uniqueName) != nullptr)
    {
        uniqueName = name + "_" + std::to_string(i);
        i++;
    }
    return uniqueName;
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
Scene::addControl(std::shared_ptr<DeviceControl> control)
{
    auto obj = std::make_shared<SceneObject>();
    obj->addComponent(control);
    addSceneObject(obj);
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
    for (const auto& ent : m_sceneEntities)
    {
        if (auto obj = std::dynamic_pointer_cast<SceneObject>(ent))
        {
            obj->reset();
        }
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

    // Process all behaviours before updating the scene.
    // This includes controls such as haptics, keyboard, mouse, etc.
    for (auto obj : this->getSceneObjects())
    {
        // SceneObject update for supporting old imstk
        if (auto sceneObj = std::dynamic_pointer_cast<SceneObject>(obj))
        {
            sceneObj->update();
        }
        for (auto comp : obj->getComponents())
        {
            if (auto behaviour = std::dynamic_pointer_cast<SceneBehaviour>(comp))
            {
                behaviour->update(dt);
            }
        }
    }

    // Execute the computational graph
    if (m_taskGraphController != nullptr)
    {
        m_taskGraphController->execute();
    }

    m_sceneTime += dt;
    if (m_resetRequested)
    {
        resetSceneObjects();
        m_sceneTime      = 0.0;
        m_resetRequested = false;
    }

    // FPS of physics is given by the measured time, not the given time step dt
    const double elapsedTime = wwt.getTimeElapsed(StopWatch::TimeUnitType::seconds);
    m_fps = 1.0 / elapsedTime;
    m_frameTimes.pushBack(elapsedTime);

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
Scene::updateVisuals(const double dt)
{
    for (const auto& ent : m_sceneEntities)
    {
        if (auto obj = std::dynamic_pointer_cast<SceneObject>(ent))
        {
            obj->visualUpdate();
        }
        for (auto comp : ent->getComponents())
        {
            if (auto behaviour = std::dynamic_pointer_cast<SceneBehaviour>(comp))
            {
                behaviour->visualUpdate(dt);
            }
        }
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