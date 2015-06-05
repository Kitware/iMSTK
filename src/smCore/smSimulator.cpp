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

#include "smCore/smSimulator.h"

/// \brief starts the tasks with the threads from thread pool
void smSimulator::beginFrame()
{
    frameCounter++;
}

/// \brief waits until the frame ends
void smSimulator::endFrame()
{

}

void smSimulator::initAsyncThreadPool()
{
    asyncThreadPoolSize = 0;

    for (size_t i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            asyncThreadPoolSize++;
        }
    }

    asyncPool = make_unique<ThreadPool>(asyncThreadPoolSize);
}

/// \brief the main simulation loop
void smSimulator::run()
{
    std::vector< std::future<int> > results;
    std::vector< std::future<int> > asyncResults;
    std::shared_ptr<smObjectSimulator> objectSimulator;

    if (isInitialized == false)
    {
        log->addError("Simulator is not initialized.");
        return;
    }

    results.reserve(this->simulators.size()); //make space for results
    smSimulationMainParam param;
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

    while (true && this->terminateExecution == false)
    {
        beginModule();

        if (main != NULL)
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
        std::shared_ptr<smCollisionDetection> collisionDetection;
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

        results.clear(); //clear the results buffer for new
        std::shared_ptr<smContactHandling> contactHandling;
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
void smSimulator::registerObjectSimulator(std::shared_ptr<smObjectSimulator> objectSimulator)
{
    simulators.emplace_back(objectSimulator);
    objectSimulator->enabled = true;
}

/// \brief
void smSimulator::registerCollisionDetection(std::shared_ptr<smCollisionDetection> p_collisionDetection)
{
    collisionDetectors.emplace_back(p_collisionDetection);
}

/// \brief
void smSimulator::registerSimulationMain(std::shared_ptr<smSimulationMain> p_main)
{
    changedMain = p_main;
    this->changedMainTimeStamp++;
}

void smSimulator::init()
{
    if(isInitialized == true)
    {
        return;
    }
    if(maxThreadCount == 0)
    {
        maxThreadCount = std::max(simulators.size(), collisionDetectors.size());
    }
    threadPool = make_unique<ThreadPool>(maxThreadCount);

    for(size_t i = 0; i < this->simulators.size(); i++)
    {
        simulators[i]->init();
    }

    initAsyncThreadPool();
    isInitialized = true;
}

smSimulator::smSimulator(std::shared_ptr< smErrorLog > p_log)
{
    type = SIMMEDTK_SMSIMULATOR;
    isInitialized = false;
    log = p_log;
    frameCounter = 0;
    main = nullptr;
    changedMain = nullptr;
    changedMainTimeStamp = 0;
    mainTimeStamp = 0;
    maxThreadCount = 0;
    asyncThreadPoolSize = 0;
}

void smSimulator::setMaxThreadCount(int p_threadMaxCount)
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

void smSimulator::exec()
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
}
