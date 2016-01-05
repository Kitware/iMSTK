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

#include "SimulationManager/Simulator.h"
#include "Core/MakeUnique.h"

// Threads includes
#include <ThreadPool.h>

/// \brief starts the tasks with the threads from thread pool
void Simulator::beginFrame()
{
    frameCounter++;
}

/// \brief waits until the frame ends
void Simulator::endFrame()
{

}

void Simulator::initAsyncThreadPool()
{
    asyncThreadPoolSize = 0;

    for (size_t i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            asyncThreadPoolSize++;
        }
    }

    asyncPool = Core::make_unique<ThreadPool>(asyncThreadPoolSize);
}

/// \brief the main simulation loop
void Simulator::run()
{
    std::vector< std::future<int> > results;
    std::vector< std::future<int> > asyncResults;
    std::shared_ptr<ObjectSimulator> objectSimulator;

    if (isInitialized == false)
    {
        log->addError("Simulator is not initialized.");
        return;
    }

    results.reserve(this->simulators.size()); //make space for results
    SimulationMainParam param;
    param.sceneList = sceneList;

    //Start up async threads
    asyncResults.reserve(this->asyncThreadPoolSize);
    for (size_t i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            objectSimulator = simulators[i];
            asyncResults.emplace_back(asyncPool->enqueue(
                [objectSimulator]()
                {
                    objectSimulator->run();
                    return 0; //this return is just so we have a results value
                })
            );
        }
    }

    while (!this->terminateExecution)
    {
        beginModule();

        if (main != nullptr)
        {
            main->simulateMain(param);
        }

        if (changedMainTimeStamp > mainTimeStamp)
        {
            main = changedMain;
            changedMainTimeStamp = mainTimeStamp;
        }

        results.clear();
        for (size_t i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];

            if (objectSimulator->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
            {
                continue;
            }

            if (objectSimulator->enabled == false)
            {
                continue;
            }

            //start each simulator in it's own thread (as max threads allow...)
            results.emplace_back(threadPool->enqueue(
                [objectSimulator]()
                {
                    objectSimulator->run();
                    return 0; //this return is just so we have a results value
                })
            );
        }

        for (auto&& result : results)
        { //Wait until there is a valid return value from each thread
            result.get(); //waits for result value
        }

        for (size_t i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];
            objectSimulator->syncBuffers();
        }

        results.clear(); //clear the results buffer for new
        std::shared_ptr<CollisionDetection> collisionDetection;
        for (size_t i = 0; i < this->collisionDetectors.size(); i++)
        {
            collisionDetection = collisionDetectors[i];
            //start each simulator in it's own thread (as max threads allow...)

            for(const auto &pair : collisionPairs)
            {
                results.emplace_back(threadPool->enqueue(
                    [collisionDetection,pair]()
                    {
                        collisionDetection->computeCollision(pair);
                        return 0; //this return is just so we have a results value
                    })
                );
            }
        }

        for (auto&& result : results)
        { //Wait until there is a valid return value from each thread
            result.get(); //waits for result value
        }

        //for (const auto &x : contactHandlers)
        //{
        //    results.emplace_back(threadPool->enqueue(
        //        [x]()
        //    {
        //        x->resolveContacts();
        //        return 0; //this return is just so we have a results value
        //    })
        //    );
        //}

        results.clear(); //clear the results buffer for new
        std::shared_ptr<ContactHandling> contactHandling;
        for (size_t i = 0; i < this->contactHandlers.size(); i++)
        {
            contactHandlers[i]->resolveContacts();
        }


        for (auto&& result : results)
        { //Wait until there is a valid return value from each thread
            result.get(); //waits for result value
        }

        endModule();
    }

    for (auto&& result : asyncResults)
    { //Wait until there is a valid return value from each thread
        result.get(); //waits for result value
    }

}

/// \brief
void Simulator::registerObjectSimulator(std::shared_ptr<ObjectSimulator> objectSimulator)
{
    simulators.emplace_back(objectSimulator);
    objectSimulator->enabled = true;
}

/// \brief
void Simulator::registerCollisionDetection(std::shared_ptr<CollisionDetection> p_collisionDetection)
{
    collisionDetectors.emplace_back(p_collisionDetection);
}

/// \brief
void Simulator::registerContactHandling(std::shared_ptr<ContactHandling> p_contactHandling)
{
    contactHandlers.emplace_back(p_contactHandling);
}

/// \brief
void Simulator::registerSimulationMain(std::shared_ptr<SimulationMain> p_main)
{
    changedMain = p_main;
    this->changedMainTimeStamp++;
}

bool Simulator::init()
{
    if(isInitialized == true)
    {
        return false;
    }
    if(maxThreadCount == 0)
    {
        maxThreadCount = std::max(simulators.size(), collisionDetectors.size());
    }
    threadPool = Core::make_unique<ThreadPool>(maxThreadCount);

    for(size_t i = 0; i < this->simulators.size(); i++)
    {
        simulators[i]->init();
    }

    initAsyncThreadPool();
    isInitialized = true;

    return true;
}

Simulator::Simulator(std::shared_ptr< ErrorLog > p_log)
{
    type = core::ClassType::Simulator;
    isInitialized = false;
    log = p_log;
    frameCounter = 0;
    main = nullptr;
    changedMain = nullptr;
    changedMainTimeStamp = 0;
    mainTimeStamp = 0;
    maxThreadCount = 0;
    asyncThreadPoolSize = 0;
    this->name = "Simulator";
}

void Simulator::setMaxThreadCount(int p_threadMaxCount)
{
    if(p_threadMaxCount < 0)
    {
        return;
    }
    else
    {
        maxThreadCount = p_threadMaxCount;
    }
}

void Simulator::exec()
{
    if(isInitialized)
    {
        run();
    }
    else
    {
        init();
        run();
    }

    this->terminationCompleted = true;
    std::cout << "Simulator terminated" <<std::endl;
}
void Simulator::addCollisionPair(std::shared_ptr< CollisionPair > pair)
{
    collisionPairs.emplace_back(pair);
}
