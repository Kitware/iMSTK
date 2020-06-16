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

#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace imstk
{
class Camera;
class CameraController;
class CollisionGraph;
class IBLProbe;
class Light;
class SceneObject;
class SceneObjectControllerBase;
class TaskGraph;
class TaskGraphController;
class VisualModel;

namespace ParallelUtils { class SpinLock; }

enum class TimeSteppingPolicy
{
    AsFastAsPossible,
    FixedFrameRate,
    RealTime
};

struct SceneConfig
{
    // Initializes the scene only when it needs to frame
    // Note: May cause delays to run the first frame of the scene due to scene initialization
    bool lazyInitialization = false;

    TimeSteppingPolicy timeStepping = TimeSteppingPolicy::AsFastAsPossible;

    // Keep track of the fps for the scene
    bool trackFPS = false;

    // If off, tasks will run sequentially
    bool taskParallelizationEnabled = true;

    // If on, elapsed times for computational steps will be reported in map
    bool taskTimingEnabled = false;

    // If on, the task graph will be written to a file
    bool writeTaskGraph = false;

    // If on, non functional nodes and redundant edges will be removed from final graph
    bool graphReductionEnabled = true;
};

///
/// \class Scene
///
/// \brief
///
class Scene
{
template<class T>
using NamedMap = std::unordered_map<std::string, std::shared_ptr<T>>;

public:
    ///
    /// \brief Constructor
    ///
    explicit Scene(const std::string& name, std::shared_ptr<SceneConfig> config = std::make_shared<SceneConfig>());

    ///
    /// \brief Destructor
    ///
    ~Scene();

public:
    ///
    /// \brief Initialize the scene
    ///
    bool initialize();

    ///
    /// \brief Setup the task graph, this completely rebuilds the graph
    ///
    void buildTaskGraph();

    ///
    /// \brief Intializes the graph after its in a built state
    ///
    void initTaskGraph();

    ///
    /// \brief Launch camera controller and other scene specific modules that need to run independently
    ///
    void launchModules();

    ///
    /// \brief Reset the scene
    ///
    void reset();
    void resetSceneObjects();

    ///
    /// \brief Advance the scene from current to next frame
    ///
    void advance();

    ///
    /// \brief Advance the scene from current to next frame with specified timestep
    ///
    void advance(const double dt);

    ///
    /// \brief Returns true if the object with a given name is registered, else false
    ///
    bool isObjectRegistered(const std::string& sceneObjectName) const;

    ///
    /// \brief Return a vector of shared pointers to the scene objects
    /// NOTE: A separate list might be efficient as this is called runtime
    ///
    const std::vector<std::shared_ptr<SceneObject>> getSceneObjects() const;

    ///
    /// \brief Return a vector of shared pointers to the scene objects
    /// NOTE: A separate list might be efficient as this is called runtime
    ///
    const std::vector<std::shared_ptr<VisualModel>> getDebugRenderModels() const;

    ///
    /// \brief Get the scene object controllers
    ///
    const std::vector<std::shared_ptr<SceneObjectControllerBase>> getSceneObjectControllers() const;

    ///
    /// \brief Get a scene object of a specific name
    ///
    std::shared_ptr<SceneObject> getSceneObject(const std::string& sceneObjectName) const;

    ///
    /// \brief Add/remove a scene object
    ///
    void addSceneObject(std::shared_ptr<SceneObject> newSceneObject);
    void removeSceneObject(const std::string& sceneObjectName);

    ///
    /// \brief Add a debug visual model object
    ///
    void addDebugVisualModel(std::shared_ptr<VisualModel> dbgRenderModel);

    ///
    /// \brief
    ///
    bool isLightRegistered(const std::string& lightName) const;

    ///
    /// \brief Return a vector of lights in the scene
    ///
    const std::vector<std::shared_ptr<Light>> getLights() const;

    ///
    /// \brief Get a light with a given name
    ///
    std::shared_ptr<Light> getLight(const std::string& lightName) const;

    ///
    /// \brief Add/remove lights from the scene
    ///
    void addLight(std::shared_ptr<Light> newLight);
    void removeLight(const std::string& lightName);

    ///
    /// \brief Add/remove lights from the scene
    ///
    void setGlobalIBLProbe(std::shared_ptr<IBLProbe> newIBLProbe);
    std::shared_ptr<IBLProbe> getGlobalIBLProbe();

    ///
    /// \brief Get the name of the scene
    ///
    const std::string& getName() const;

    ///
    /// \brief Get the computational graph of the scene
    ///
    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

    ///
    /// \brief Get the camera for the scene
    ///
    std::shared_ptr<Camera> getCamera() const;

    ///
    /// \brief Return the collision graph
    ///
    std::shared_ptr<CollisionGraph> getCollisionGraph() const;

    ///
    /// \brief Add objects controllers
    ///
    void addObjectController(std::shared_ptr<SceneObjectControllerBase> controller);

    ///
    /// \brief Add objects controllers
    ///
    void addCameraController(std::shared_ptr<CameraController> camController);

    ///
    /// \brief
    ///
    bool isInitialized() const { return m_isInitialized; }

    ///
    /// \brief Set/Get the FPS
    ///
    void setFPS(const double fps) { m_fps = fps; }
    double getFPS() { return m_fps; }

    ///
    /// \brief Get the elapsed time
    ///
    double getElapsedTime() { return m_elapsedTime; }

    ///
    /// \brief Get the elapsed time of a particular step
    ///
    double getElapsedTime(const std::string& stepName) const;

    ///
    /// \brief Get the map of elapsed times
    ///
    const std::unordered_map<std::string, double>& getTaskComputeTimes() const { return m_nodeComputeTimes; }

    ///
    /// \brief Lock the compute times resource
    ///
    void lockComputeTimes();

    ///
    /// \brief Unlock the compute times resource
    ///
    void unlockComputeTimes();

    ///
    /// \brief Called after compute graph is built, but before initialized
    ///
    void setTaskGraphConfigureCallback(std::function<void(Scene*)> callback) { this->m_postTaskGraphConfigureCallback = callback; }

    ///
    /// \brief Get the configuration
    ///
    std::shared_ptr<const SceneConfig> getConfig() const { return m_config; };
    const std::shared_ptr<SceneConfig> getConfig() { return m_config; };

protected:
    std::shared_ptr<SceneConfig> m_config;

    std::string m_name;                              ///> Name of the scene
    NamedMap<SceneObject>           m_sceneObjectsMap;
    NamedMap<VisualModel>           m_DebugRenderModelMap;
    NamedMap<Light>                 m_lightsMap;
    std::shared_ptr<IBLProbe>       m_globalIBLProbe = nullptr;
    std::shared_ptr<Camera>         m_camera = nullptr;
    std::shared_ptr<CollisionGraph> m_collisionGraph = nullptr;
    std::vector<std::shared_ptr<SceneObjectControllerBase>> m_objectControllers; ///> List of object controllers
    std::vector<std::shared_ptr<CameraController>> m_cameraControllers;          ///> List of camera controllers
    std::unordered_map<std::string, std::thread>   m_threadMap;                  ///>

    std::shared_ptr<TaskGraph> m_taskGraph = nullptr;                            ///> Computational graph
    std::shared_ptr<TaskGraphController> m_taskGraphController   = nullptr;      ///> Controller for the computational graph
    std::function<void(Scene*)> m_postTaskGraphConfigureCallback = nullptr;

    std::shared_ptr<ParallelUtils::SpinLock> computeTimesLock = nullptr;
    std::unordered_map<std::string, double>  m_nodeComputeTimes; ///> Map of ComputeNode names to elapsed times for benchmarking

    double m_fps = 0.0;
    double m_elapsedTime = 0.0;

    bool m_isInitialized = false;

    std::atomic<bool> m_resetRequested = ATOMIC_VAR_INIT(false);
};
} // imstk
