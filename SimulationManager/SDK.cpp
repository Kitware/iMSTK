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

#include "SimulationManager/SDK.h"

// Threads includes
#include <ThreadPool.h>

namespace imstk {

std::once_flag SDK::sdkCallOnceFlag;

//---------------------------------------------------------------------------
void SDK::initRegisteredModules()
{
    for(auto &module : this->moduleList)
    {
        module->init();
    }
}

//---------------------------------------------------------------------------
void SDK::runRegisteredModules()
{
    // Make sure modules are ran only once
    if (modulesInitialized)
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

    modulesInitialized = true;
}

//---------------------------------------------------------------------------
void SDK::initialize()
{
    InitVTKRendering();
    InitIODelegates();
    this->modulesInitialized = false;
    this->sceneList.clear();
    this->simulator = this->createSimulator();
    auto scene = this->createScene();
    this->errorLog = ErrorLog::getDefaultLogger();
    this->viewer = this->createViewer();
    if(this->viewer)
    {
        this->viewer->registerScene(scene,
                                    imstk::IMSTK_RENDERTARGET_SCREEN,
                                    "Collision pipeline demo");
    }
}

//---------------------------------------------------------------------------
std::shared_ptr<Scene> SDK::createScene()
{
    auto scene = std::make_shared<Scene>(errorLog);
    scene->setName("Scene" + std::to_string(scene->getUniqueId()->getId()));
    this->sceneList.emplace_back(scene);
    return scene;
}

//---------------------------------------------------------------------------
std::shared_ptr<ViewerBase> SDK::createViewer()
{
    this->viewer =
        Factory<ViewerBase>::createSubclassForGroup("ViewerBase",RenderDelegate::VTK);

    if(!this->viewer)
    {
        // TODO: Log this
        return nullptr;
    }

    this->addModule(this->viewer);
    return this->viewer;
}

//---------------------------------------------------------------------------
void SDK::setViewer(std::shared_ptr<ViewerBase> newViewer)
{
    if(!newViewer)
    {
        // TODO: log this
        return;
    }

    this->viewer.reset();
    this->viewer = newViewer;
}

//---------------------------------------------------------------------------
std::shared_ptr<ViewerBase> SDK::getViewer()
{
    return this->viewer;
}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
std::shared_ptr<Simulator> SDK::createSimulator()
{
    if(!this->simulator)
    {
        this->simulator = std::make_shared<Simulator>();
        this->addModule(simulator);
    }

    return simulator;
}

//---------------------------------------------------------------------------
std::shared_ptr<Simulator> SDK::getSimulator()
{
    return this->simulator;
}

//---------------------------------------------------------------------------
std::shared_ptr< Scene > SDK::getScene()
{
    return sceneList.at(0);
}

//---------------------------------------------------------------------------
std::shared_ptr<ErrorLog> SDK::getErrorLog()
{
    return errorLog;
}

//---------------------------------------------------------------------------
void SDK::terminateAll()
{
    for(auto &module : this->moduleList)
    {
        module->terminate();
        module->waitTermination();
    }
}

//---------------------------------------------------------------------------
void SDK::addModule(std::shared_ptr<Module> newModule)
{
    if(std::end(this->moduleList) ==
        std::find(std::begin(this->moduleList),std::end(this->moduleList),newModule) )
    {
        this->moduleList.emplace_back(newModule);
    }
    else
    {
        // TODO: LOg this
    }
}

//---------------------------------------------------------------------------
void SDK::addSceneActor(std::shared_ptr<SceneObject> p_sco,
                        std::shared_ptr<ObjectSimulator> p_os)
{
    if(!p_sco || !p_os)
    {
        // TODO: LOG this
        std::cerr << "Empty objects" << std::endl;
        return;
    }

    p_os->addModel(p_sco);
    this->simulator->registerObjectSimulator(p_os);
    // TODO: This only uses one scene
    this->getScene()->addSceneObject(p_sco);
}

//---------------------------------------------------------------------------
void SDK::run()
{
    this->initRegisteredModules();
    this->runRegisteredModules();
    if (this->viewer)
    {
        this->viewer->exec();
    }

    // Tell framework threads to shutdown if the viewer returns
    this->terminateAll();

    // Wait for all threads to finish processing
    for (auto &moduleThread : this->modules)
    {
        moduleThread.join();
    }
    std::cout << "Killing SDK" << std::endl;
}

//---------------------------------------------------------------------------
std::shared_ptr< DeformableSceneObject > SDK::createDeformableModel(const std::string &meshFile, const std::string &configFile)
{
    auto model = std::make_shared<VegaFEMDeformableSceneObject>(meshFile, configFile);
    auto simulator = std::make_shared<ObjectSimulator>();
    this->addSceneActor(model, simulator);
    return model;
}

//---------------------------------------------------------------------------
std::shared_ptr< StaticSceneObject > SDK::createStaticModel()
{
    auto model = std::make_shared<StaticSceneObject>();
    auto simulator = std::make_shared<ObjectSimulator>();
    this->addSceneActor(model, simulator);
    return model;
}

//---------------------------------------------------------------------------
void SDK::addInteraction(std::shared_ptr< CollisionManager > collisionPair,
                         std::shared_ptr< CollisionDetection > collisionDetection,
                         std::shared_ptr< ContactHandling > contactHandling)
{
    this->simulator->registerInteraction(collisionPair,
                                         collisionDetection,
                                         contactHandling);
}

//---------------------------------------------------------------------------
std::shared_ptr< VRPNDeviceServer > SDK::createDeviceServer()
{
    auto server = std::make_shared<imstk::VRPNDeviceServer>();
    this->addModule(server);
    return server;
}

//---------------------------------------------------------------------------
std::shared_ptr< ToolCoupler > SDK::createForceDeviceController(std::string &deviceURL, bool createServer)
{
    auto client = std::make_shared<VRPNForceDevice>(deviceURL);
    this->addModule(client);

    if(createServer)
    {
        auto server = this->createDeviceServer();
        server->addDeviceClient(client);
    }

    auto controller = std::make_shared<ToolCoupler>(client);
    this->addModule(controller);

    return controller;
}

}
