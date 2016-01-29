// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SIMULATION_MANAGER_SDK_H
#define SIMULATION_MANAGER_SDK_H

// STL includes
#include <memory>
#include <thread>
#include <mutex>
#include <vector>

// iMSTK includes
#include "Core/Module.h"
#include "Rendering/InitVTKRendering.h"
#include "IO/InitIO.h"
#include "Core/ErrorLog.h"
#include "Rendering/ViewerBase.h"
#include "Core/Factory.h"
#include "Core/RenderDelegate.h"
#include "SimulationManager/Simulator.h"
#include "Simulators/ObjectSimulator.h"
#include "SceneModels/VegaFEMDeformableSceneObject.h"
#include "SceneModels/StaticSceneObject.h"
#include "Core/ContactHandling.h"
#include "Core/CollisionManager.h"
#include "Core/CollisionDetection.h"
#include "Devices/VRPNDeviceServer.h"
#include "Devices/VRPNForceDevice.h"
#include "VirtualTools/ToolCoupler.h"

namespace imstk {

///
/// \brief SDK class. It is a Singleton implementation of the framework simulation driver.
///     Use the static method
///
///  Note: SDK::createSDK() to get the instance.
///
class SDK
{
public:
    ///
    /// \brief Constructor/Destructor
    ///
    ~SDK() = default;

    ///
    /// \brief Prevent use of assignment operator
    ///
    SDK &operator=(const SDK&) = delete;

    ///
    /// \brief Initialize modules. Run the initialization method for each module.
    ///
    void initRegisteredModules();

    ///
    /// \brief Run the registered modules. This will not run any modules that inherit
    ///     ViewerBase as on some platforms (Mac OS X) only the main thread can run user
    ///     interface code. The return value is -1 if the modules are already running or
    ///     no module inherits ViewerBase. Otherwise, the index of the last viewer module
    ///     encountered is returned.
    ///
    ///  NOTE: This function assumes that there is only one viewer.
    ///
    void runRegisteredModules();

    ///
    /// \brief Initialize the SDK.
    ///
    void initialize();

    ///
    /// \brief SDK creates scene
    ///
    std::shared_ptr<Scene> createScene();

    ///
    /// \brief Creates and registers a viewer object with the SDK
    ///
    std::shared_ptr<ViewerBase> createViewer();

    ///
    /// \brief Create a singleton SDK
    ///
    static std::shared_ptr<SDK> createSDK();

    ///
    /// \brief Replaces the internal viewer object with the SDK.
    ///
    /// \param newViewer Pointer to new external viewer
    ///
    void setViewer(std::shared_ptr<ViewerBase> newViewer);

    ///
    /// \brief Returns a pointer to the viewer object
    ///
    std::shared_ptr<ViewerBase> getViewer();

    ///
    /// \brief SDK creates simualtor
    ///
    std::shared_ptr<Simulator> createSimulator();

    ///
    /// \brief SDK creates simualtor
    ///
    std::shared_ptr<Simulator> getSimulator();

    ///
    /// \brief SDK creates scene
    ///
    std::shared_ptr<Scene> getScene();

    ///
    /// \brief SDK returns logger for the system
    ///
    std::shared_ptr<ErrorLog> getErrorLog();

    ///
    /// \brief Terminate all modules.
    ///
    void terminateAll();

    ///
    /// \brief Add a module to the module list.
    ///     This module will not be inserted if it already exist in the list.
    ///
    /// \param newModule Reference to a new module
    ///
    void addModule(std::shared_ptr<Module> newModule);

    ///
    /// \brief Adds a scene object and its simulator to the scene and simulator list
    ///     respectively. It also saves a reference to the simulator in the scene object.
    ///
    ///  \param sceneObject Scene object to attache to the scene.
    ///  \param objectSimulator Object simulator to attach to the simulator manager.
    ///
    void addSceneActor(std::shared_ptr<SceneObject> sceneObject,
                       std::shared_ptr<ObjectSimulator> objectSimulator);

    ///
    /// \brief Run registered modules and viewer.
    ///
    void run();

    ///
    /// \brief Utility function to create a FEM model.
    ///
    /// \param meshFile File containing a tetrahedral mesh.
    /// \param configFile Configuration file containing parameters for the FEM method.
    ///
    /// \see VegaFEMDeformableSceneObject
    ///
    std::shared_ptr<DeformableSceneObject> createDeformableModel(const std::string &meshFile,
                                                                 const std::string &configFile);

    ///
    /// \brief Utility function to create a Static model.
    ///
    std::shared_ptr<StaticSceneObject> createStaticModel();

    ///
    /// \brief Utility function to add an interaction to the simulator
    ///
    /// \param collisionPair Holds the collision data used by the collision handler
    /// \param collisinDetection Populates the collision pair data
    /// \param contactHandling Collision response.
    ///
    void addInteraction(std::shared_ptr<CollisionManager> collisionPair,
                        std::shared_ptr<CollisionDetection> collisionDetection,
                        std::shared_ptr<ContactHandling> contactHandling);

    ///
    /// \brief Utility function to create a device server
    ///
    std::shared_ptr<VRPNDeviceServer> createDeviceServer();

    ///
    /// \brief Utility function to create a device client, server and controller
    ///
    /// \param deviceURL Client name to make the connection
    ///
    std::shared_ptr<ToolCoupler> createForceDeviceController(std::string &deviceURL, bool createServer = false);

private:
    SDK() = default;

private:
    static std::once_flag sdkCallOnceFlag; ///< Flag to make sure that only one call of
                                           ///< the SDK singleton creator is ran to completion

    bool shutdown; ///< Tells the SDK to terminate
    bool modulesInitialized; ///< Flag to make sure modules are ran only once

    std::shared_ptr<ErrorLog> errorLog; ///< Reference to singleton error log
    std::shared_ptr<ViewerBase> viewer; ///< Reference to the sdk viewer object
    std::shared_ptr<Simulator> simulator; ///< Reference to the sdk simulator object

    std::vector<std::shared_ptr<Scene>> sceneList; ///< List of scenes containers
    std::vector<std::shared_ptr<Module>> moduleList; ///< Modules to be ran in parallel
    std::vector<std::thread> modules; ///< Stores a list of running module threads
};

}

#endif
