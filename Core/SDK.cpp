// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/SDK.h"
#include "Core/Factory.h"
#include "Core/RenderDelegate.h"

#include <chrono>
#include <thread>
#include <string>

/// \brief SDK is singlenton class
std::once_flag SDK::sdkCallOnceFlag;

/// \brief constructor
SDK::SDK()
{
    shutdown = false;
    sceneIdCounter = 1;
    isModulesStarted = false;
    type = core::ClassType::Sdk;
    viewer = nullptr;
    simulator = nullptr;
    sceneList.clear();

    errorLog = std::make_shared<ErrorLog>();
}

SDK::~SDK()
{
    std::cout << "Killing SDK" << std::endl;
}

/// \brief creates the scene of the simulator
std::shared_ptr<Scene> SDK::createScene()
{
    auto scene = std::make_shared<Scene>(errorLog);
    registerScene(scene);
    scene->setName("Scene" + std::to_string(scene->getUniqueId()->getId()));
    return scene;
}

void SDK::releaseScene(std::shared_ptr<Scene> scene)
{
    scene.reset();
}

std::shared_ptr<ViewerBase> SDK::createViewer()
{
    this->viewer =
        Factory<ViewerBase>::createSubclassForGroup("ViewerBase",RenderDelegate::VTK);
    if (this->viewer)
    {
        this->viewer->log = this->errorLog;
        this->registerModule(this->viewer);
    }
    else
    {
        std::cerr << "Error: Unable to create viewer." << std::endl;
    }

    return this->viewer;
}

void SDK::addViewer(std::shared_ptr<ViewerBase> p_viewer)
{
    assert(p_viewer);

    this->viewer.reset();
    this->viewer = p_viewer;
    this->viewer->log = this->errorLog;
    this->registerModule(p_viewer);
}

std::shared_ptr<ViewerBase> SDK::getViewerInstance()
{
    return this->viewer;
}

/// \brief
std::shared_ptr<Simulator> SDK::createSimulator()
{
    if(!this->simulator)
    {
        simulator = std::make_shared<Simulator>(errorLog);
        simulator->sceneList = this->sceneList;
        registerModule(simulator);
    }

    return simulator;
}

/// \brief
void SDK::updateSceneListAll()
{
}

/// \brief Initialize all modules registered to the SimMedTK SDK
void SDK::initRegisteredModules()
{
    for(auto &module : this->moduleList)
    {
        if(module->getType() != core::ClassType::Viewer)
        {
            module->init();
        }
    }
}

/** \brief Run the registered modules
  *
  * This will not run any modules that inherit ViewerBase as
  * on some platforms (Mac OS X) only the main thread can run
  * user interface code. The return value is -1 if the modules
  * are already running or no module inherits ViewerBase.
  * Otherwise, the index of the last viewer module encountered
  * is returned.
  * Note: This function assumes that there is only one viewer.
  */
void SDK::runRegisteredModules()
{
    if (isModulesStarted)
    {
        return;
    }

    for(auto &module : this->moduleList)
    {
        auto viewer = std::dynamic_pointer_cast<ViewerBase>(module);
        if(!viewer)
        {
            modules.emplace_back([module]{module->exec();});
        }
    }

    isModulesStarted = true;
}

///\brief shutdowns all the modules
void SDK::shutDown()
{
    for(auto &module : this->moduleList)
    {
        module->terminateExecution = true;
    }
    shutdown = true;
}

/// \brief runs the simulator
void SDK::run()
{
    updateSceneListAll();
    initRegisteredModules();

    runRegisteredModules();

    if (nullptr != this->viewer)
    {
        this->viewer->exec();

        // Now wait for other modules to shut down
        while (this->viewer->isValid() && !shutdown)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else
    {
        // Now wait for other modules to shut down
        while (!shutdown)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Tell framework threads to shutdown
    terminateAll();

    // Wait for all threads to finish processing
    for (auto &module : modules)
    {
        module.join();
    }
}

/// \brief
void SDK::addRef(std::shared_ptr<CoreClass> p_coreClass)
{
    ++*p_coreClass;
}

/// \brief
void SDK::removeRef(std::shared_ptr<CoreClass> p_coreClass)
{
    --*p_coreClass;
}

std::shared_ptr<SDK> SDK::createSDK()
{
    static std::shared_ptr<SDK> sdk; ///< singleton sdk.
    std::call_once(sdkCallOnceFlag,
                   []
                    {
                        sdk.reset(new SDK);
                    });
    return sdk;
}

std::shared_ptr<SDK> SDK::createStandardSDK()
{
    auto sdk = createSDK();

    sdk->createScene();
    sdk->createViewer();

    sdk->createSimulator();

    return sdk;
}

std::shared_ptr<SDK> SDK::getInstance()
{
    return SDK::createSDK();
}

void SDK::terminateAll()
{
    for(auto &module : this->moduleList)
    {
        module->terminateExecution = true;
    }

    for(auto &module : this->moduleList)
    {
        bool moduleTerminated = false;
        while(!moduleTerminated)
        {
            if(module->isTerminationDone())
            {
                moduleTerminated = true;
            }
        }
    }
}

/// \brief register functions
void SDK::registerMesh(std::shared_ptr<Core::BaseMesh> newMesh)
{
    if(std::end(this->meshList) ==
        std::find(std::begin(this->meshList),std::end(this->meshList),newMesh) )
    {
        this->meshList.emplace_back(newMesh);
    }
}

void SDK::registerModule(std::shared_ptr<Module> newModule)
{
    if(std::end(this->moduleList) ==
        std::find(std::begin(this->moduleList),std::end(this->moduleList),newModule) )
    {
        this->moduleList.emplace_back(newModule);
    }
}

void SDK::registerObjectSimulator(std::shared_ptr<ObjectSimulator> newObjectSimulator)
{
    if(std::end(this->simulatorList) ==
        std::find(std::begin(this->simulatorList),std::end(this->simulatorList),newObjectSimulator) )
    {
        this->simulatorList.emplace_back(newObjectSimulator);
    }
}

void SDK::registerCollisionDetection(std::shared_ptr<CollisionDetection> newCollisionDetection)
{
    if(std::end(this->collisionDetectionList) ==
        std::find(std::begin(this->collisionDetectionList),std::end(this->collisionDetectionList),newCollisionDetection) )
    {
        this->collisionDetectionList.emplace_back(newCollisionDetection);
    }
}

void SDK::registerScene(std::shared_ptr<Scene> newScene)
{
    if(std::end(this->sceneList) ==
        std::find(std::begin(this->sceneList),std::end(this->sceneList),newScene) )
    {
        this->sceneList.emplace_back(newScene);
    }
}

void SDK::registerSceneObject(std::shared_ptr<SceneObject> newSceneObject)
{
    if(std::end(this->sceneObjectList) ==
        std::find(std::begin(this->sceneObjectList),std::end(this->sceneObjectList),newSceneObject) )
    {
        this->sceneObjectList.emplace_back(newSceneObject);
    }
}

void SDK::addSceneActor(std::shared_ptr<SceneObject> p_sco, std::shared_ptr<ObjectSimulator> p_os, int p_scId)
{
    if(!p_sco || !p_os)
    {
        std::cerr << "Empty objects" << std::endl;
        return;
    }

    p_sco->attachObjectSimulator(p_os);

    this->registerObjectSimulator(p_os);

    this->registerSceneObject(p_sco);

    this->getScene(p_scId)->addSceneObject(p_sco);
}

///SDK returns logger for the system
std::shared_ptr<ErrorLog> SDK::getErrorLog()
{
    return errorLog;
};

std::shared_ptr<Simulator> SDK::getSimulator()
{
    return this->simulator;
}
