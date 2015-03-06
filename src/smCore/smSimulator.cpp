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

    for (smInt i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            asyncThreadPoolSize++;
        }
    }

    asyncPool = std::unique_ptr<ThreadPool>(new ThreadPool(asyncThreadPoolSize));
}

/// \brief the main simulation loop
void smSimulator::run()
{
    std::vector< std::future<int> > results;
    std::vector< std::future<int> > asyncResults;
    smObjectSimulator *objectSimulator;

    if (isInitialized == false)
    {
        log->addError(this, "Simulator is not initialized.");
        return;
    }

    results.reserve(this->simulators.size()); //make space for results
    smSimulationMainParam param;
    param.sceneList = sceneList;

    //Start up async threads
    asyncResults.reserve(this->asyncThreadPoolSize);
    for (smInt i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->execType == SIMMEDTK_SIMEXECUTION_ASYNCMODE)
        {
            objectSimulator = simulators[i];
            asyncResults.emplace_back(asyncPool->enqueue([objectSimulator]()
                {
                    objectSimulator->run();
                    return 0; //this return is just so we have a results value
                }));
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
        for (smInt i = 0; i < this->simulators.size(); i++)
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
            results.emplace_back(threadPool->enqueue([objectSimulator]()
                {
                    objectSimulator->run();
                    return 0; //this return is just so we have a results value
                }));
        }

        for (auto&& result : results)
        { //Wait until there is a valid return value from each thread
            result.get(); //waits for result value
        }

        for (smInt i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];
            objectSimulator->syncBuffers();
        }

        results.clear(); //clear the results buffer for new
        for (smInt i = 0; i < this->collisionDetectors.size(); i++)
        {
            objectSimulator = collisionDetectors[i];
            //start each simulator in it's own thread (as max threads allow...)
            results.emplace_back(threadPool->enqueue([objectSimulator]()
                {
                    objectSimulator->run();
                    return 0; //this return is just so we have a results value
                }));
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
void smSimulator::registerObjectSimulator(smObjectSimulator *objectSimulator)
{

    simulators.push_back(objectSimulator);
    objectSimulator->enabled = true;
}

/// \brief
void smSimulator::registerCollisionDetection(smObjectSimulator *p_collisionDetection)
{

    collisionDetectors.push_back(p_collisionDetection);
}

/// \brief
void smSimulator::registerSimulationMain(smSimulationMain*p_main)
{

    changedMain = p_main;
    this->changedMainTimeStamp++;
}
