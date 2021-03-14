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

#include <atomic>
#include <string>
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
class TaskGraph;
class TaskGraphController;
class TrackingDeviceControl;
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

    // If on, debug camera is positioned at scene bounding box
    bool debugCamBoundingBox = true;
};

///
/// \class Scene
///
/// \brief
///
class Scene : public EventObject
{
template<class T>
using NamedMap = std::unordered_map<std::string, std::shared_ptr<T>>;

public:
    Scene(const std::string& name, std::shared_ptr<SceneConfig> config = std::make_shared<SceneConfig>());
    virtual ~Scene() override= default;

public:
    SIGNAL(Scene,configureTaskGraph);

public:
    ///
    /// \brief Initialize the scene
    ///
    virtual bool initialize();

    ///
    /// \brief Setup the task graph, this completely rebuilds the graph
    ///
    void buildTaskGraph();

    ///
    /// \brief Intializes the graph after its in a built state
    ///
    void initTaskGraph();

    ///
    /// \brief Reset the scene
    ///
    void reset();
    void resetSceneObjects();

    ///
    /// \brief Advance the scene from current to next frame with specified timestep
    ///
    virtual void advance(const double dt);

    ///
    /// \brief Update visuals of all sceneobjects
    ///
    virtual void updateVisuals();

    ///
    /// \brief Returns true if the object with a given name is registered, else false
    ///
    bool isObjectRegistered(const std::string& sceneObjectName) const;

    ///
    /// \brief If true, tasks will be time and a table produced
    /// every scene advance of the times
    ///
    void setEnableTaskTiming(bool enabled);

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
    const std::vector<std::shared_ptr<TrackingDeviceControl>> getControllers() const{ return m_trackingControllers; }

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
    void setGlobalIBLProbe(std::shared_ptr<IBLProbe> newIBLProbe) { m_globalIBLProbe = newIBLProbe; }
    std::shared_ptr<IBLProbe> getGlobalIBLProbe() { return m_globalIBLProbe; }

    ///
    /// \brief Get the name of the scene
    ///
    const std::string& getName() const{ return m_name; }

    ///
    /// \brief Get the computational graph of the scene
    ///
    std::shared_ptr<TaskGraph> getTaskGraph() const{ return m_taskGraph; }

    ///
    /// \brief Get the camera for the scene
    ///
    std::shared_ptr<Camera> getActiveCamera() const{ return m_activeCamera; }

    ///
    /// \brief Get the name of the camera, if it exists
    ///
    std::string getCameraName(std::shared_ptr<Camera> cam) const
    {
        auto i = std::find_if(m_cameras.begin(),m_cameras.end(),
            [&cam](const NamedMap<Camera>::value_type& j) { return j.second == cam; });
        if (i != m_cameras.end())
        {
            return i->first;
        }
        else
        {
            return "";
        }
    }

    ///
    /// \brief Get camera by name
    ///
    std::shared_ptr<Camera> getCamera(std::string name) const
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

    ///
    /// \brief Set the camera for the scene
    ///
    void addCamera(std::string name,std::shared_ptr<Camera> cam) { m_cameras[name] = cam; }

    ///
    /// \brief Set the active camera by name
    ///
    void setActiveCamera(std::string name)
    {
        auto i = m_cameras.find(name);
        if (i != m_cameras.end())
        {
            m_activeCamera = m_cameras[name];
        }
    }

    ///
    /// \brief Return the collision graph
    ///
    std::shared_ptr<CollisionGraph> getCollisionGraph() const{ return m_collisionGraph; }

    ///
    /// \brief Add objects controllers
    ///
    void addController(std::shared_ptr<TrackingDeviceControl> controller);

    ///
    /// \brief Set/Get the FPS
    ///
    double getFPS() const{ return m_fps; }

    ///
    /// \brief Get the map of elapsed times
    ///
    const std::unordered_map<std::string,double>& getTaskComputeTimes() const{ return m_nodeComputeTimes; }

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
    std::shared_ptr<SceneConfig> getConfig() const{ return m_config; };

protected:
    std::shared_ptr<SceneConfig> m_config;

    std::string m_name;                              ///> Name of the scene
    NamedMap<SceneObject>     m_sceneObjectsMap;
    NamedMap<VisualModel>     m_DebugRenderModelMap;
    NamedMap<Light>           m_lightsMap;
    std::shared_ptr<IBLProbe> m_globalIBLProbe = nullptr;

    NamedMap<Camera> m_cameras;
    std::shared_ptr<Camera> m_activeCamera;

    std::shared_ptr<CollisionGraph> m_collisionGraph;
    std::vector<std::shared_ptr<TrackingDeviceControl>> m_trackingControllers; ///> List of object controllers

    std::shared_ptr<TaskGraph> m_taskGraph;                                    ///> Computational graph
    std::shared_ptr<TaskGraphController> m_taskGraphController   = nullptr;    ///> Controller for the computational graph
    std::function<void(Scene*)> m_postTaskGraphConfigureCallback = nullptr;

    std::shared_ptr<ParallelUtils::SpinLock> m_computeTimesLock;
    std::unordered_map<std::string,double>   m_nodeComputeTimes; ///> Map of ComputeNode names to elapsed times for benchmarking

    double m_fps = 0.0;

    std::atomic<bool> m_resetRequested = ATOMIC_VAR_INIT(false);
};
} // imstk
