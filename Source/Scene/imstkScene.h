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

#include "imstkEventObject.h"
#include "imstkMath.h"

#include <atomic>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace imstk
{
class Camera;
class CameraController;
class CollisionGraph;
class IBLProbe;
class Light;
class SceneObject;
class TaskGraph;
class TaskGraphController;
class TrackingDeviceControl;
class VisualModel;

namespace ParallelUtils { class SpinLock; }

struct SceneConfig
{
    // Initializes the scene only when it needs to frame
    // Note: May cause delays to run the first frame of the scene due to scene initialization
    bool lazyInitialization = false;

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

    // If on, debug camera is positioned at scene bounding box
    bool debugCamBoundingBox = true;
};

///
/// \class Scene
///
/// \brief A collection of SceneObjects and interactions
///
class Scene : public EventObject
{
template<class T>
using NamedMap = std::unordered_map<std::string, std::shared_ptr<T>>;

public:
    Scene(const std::string& name, std::shared_ptr<SceneConfig> config = std::make_shared<SceneConfig>());
    virtual ~Scene() override = default;

public:
    // *INDENT-OFF*
    SIGNAL(Scene, configureTaskGraph);
    ///
    /// \brief Called when scene is modified, when scene objects
    /// or interactions are added/removed
    ///
    SIGNAL(Scene, modified);
    // *INDENT-ON*

public:
    ///
    /// \brief Initialize the scene
    ///
    virtual bool initialize();

    ///
    /// \brief Compute the bounding box of the scene as an union of bounding boxes of its objects
    ///
    void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0);

    ///
    /// \brief Setup the task graph, this completely rebuilds the graph
    ///
    void buildTaskGraph();

    ///
    /// \brief Initializes the graph after its in a built state
    ///
    void initTaskGraph();

    ///
    /// \brief Async reset the scene, will reset next update
    ///
    void reset();

    ///
    /// \brief Sync reset, resets immediately
    ///
    void resetSceneObjects();

    ///
    /// \brief Advance the scene from current to next frame with specified timestep
    ///
    virtual void advance(const double dt);

    ///
    /// \brief Update visuals of all scene objects
    ///
    virtual void updateVisuals();

    ///
    /// \brief If true, tasks will be time and a table produced
    /// every scene advance of the times
    ///
    void setEnableTaskTiming(const bool enabled);

    ///
    /// \brief Return the SceneObjects of the scene
    ///
    const std::unordered_set<std::shared_ptr<SceneObject>>& getSceneObjects() const { return m_sceneObjects; }

    ///
    /// \brief Get the scene object controllers
    ///
    const std::vector<std::shared_ptr<TrackingDeviceControl>> getControllers() const { return m_trackingControllers; }

    ///
    /// \brief Get SceneObject by name, returns nullptr if doesn't exist
    ///
    std::shared_ptr<SceneObject> getSceneObject(const std::string& sceneObjectName) const;

    ///
    /// \brief Add/remove a scene object
    ///
    void addSceneObject(std::shared_ptr<SceneObject> newSceneObject);

    ///
    /// \brief Remove scene object by name
    ///
    void removeSceneObject(const std::string& name);

    ///
    /// \brief Remove scene object
    ///
    void removeSceneObject(std::shared_ptr<SceneObject> sceneObject);

    ///
    /// \brief Return a vector of lights in the scene
    ///
    const std::vector<std::shared_ptr<Light>> getLights() const;

    ///
    /// \brief Get a light with a given name
    ///
    std::shared_ptr<Light> getLight(const std::string& lightName) const;

    ///
    /// \brief Get and unordered map of cameras with names
    ///
    const NamedMap<Camera>& getCameras() const { return m_cameras; }

    ///
    /// \brief Add light from the scene
    ///
    void addLight(const std::string& name, std::shared_ptr<Light> newLight);

    ///
    /// \brief Remove light with a given name from the scene
    ///
    void removeLight(const std::string& lightName);

    ///
    /// \brief Set global IBL probe
    ///
    void setGlobalIBLProbe(std::shared_ptr<IBLProbe> newIBLProbe) { m_globalIBLProbe = newIBLProbe; }

    ///
    /// \brief Return global IBL probe
    ///
    std::shared_ptr<IBLProbe> getGlobalIBLProbe() { return m_globalIBLProbe; }

    ///
    /// \brief Get the name of the scene
    ///
    const std::string& getName() const { return m_name; }

    ///
    /// \brief Get the computational graph of the scene
    ///
    std::shared_ptr<TaskGraph> getTaskGraph() const { return m_taskGraph; }

    ///
    /// \brief Get the active camera for the scene
    ///
    std::shared_ptr<Camera> getActiveCamera() const { return m_activeCamera; }

    ///
    /// \brief Get the name of the camera given the object (if it exists)
    ///
    std::string getCameraName(const std::shared_ptr<Camera> cam) const;

    ///
    /// \brief Get camera object given the name
    ///
    std::shared_ptr<Camera> getCamera(const std::string name) const;

    ///
    /// \brief Set the camera for the scene
    ///
    void addCamera(const std::string& name, std::shared_ptr<Camera> cam);

    ///
    /// \brief Switch the active camera to the one requested by name.
    /// If the requested on doesn't exist, previous on remains
    ///
    void setActiveCamera(const std::string name);

    ///
    /// \brief Remove the camera with a given name
    ///
    void removeCamera(const std::string name);

    ///
    /// \brief Return the collision graph of the scene
    ///
    std::shared_ptr<CollisionGraph> getCollisionGraph() const { return m_collisionGraph; }

    ///
    /// \brief Add objects controller
    ///
    void addController(std::shared_ptr<TrackingDeviceControl> controller);

    ///
    /// \brief Set/Get the frames per second (FPS)
    ///
    double getFPS() const { return m_fps; }

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
    /// \brief Get the configuration
    ///
    std::shared_ptr<SceneConfig> getConfig() const { return m_config; };

protected:
    std::shared_ptr<SceneConfig> m_config;

    std::string m_name; ///> Name of the scene
    std::unordered_set<std::shared_ptr<SceneObject>> m_sceneObjects;
    NamedMap<Light> m_lightsMap;
    std::shared_ptr<IBLProbe> m_globalIBLProbe = nullptr;

    NamedMap<Camera> m_cameras;
    std::shared_ptr<Camera> m_activeCamera;

    std::shared_ptr<CollisionGraph> m_collisionGraph;
    std::vector<std::shared_ptr<TrackingDeviceControl>> m_trackingControllers; ///> List of object controllers

    std::shared_ptr<TaskGraph> m_taskGraph;                                    ///> Computational graph
    std::shared_ptr<TaskGraphController> m_taskGraphController   = nullptr;    ///> Controller for the computational graph
    std::function<void(Scene*)> m_postTaskGraphConfigureCallback = nullptr;

    std::shared_ptr<ParallelUtils::SpinLock> m_computeTimesLock;
    std::unordered_map<std::string, double>  m_nodeComputeTimes; ///> Map of ComputeNode names to elapsed times for benchmarking

    double m_fps = 0.0;

    std::atomic<bool> m_resetRequested = ATOMIC_VAR_INIT(false);
};
} // imstk
