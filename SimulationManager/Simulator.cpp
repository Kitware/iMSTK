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

#include "SimulationManager/Simulator.h"
#include "Core/MakeUnique.h"

#include "Core/CollisionDetection.h"
#include "Core/ContactHandling.h"
#include "Assembler/Assembler.h"
#include "Simulators/ObjectSimulator.h"
#include "CollisionContext/CollisionContext.h"
#include "Core/CollisionManager.h"

// Threads includes
#include <ThreadPool.h>

namespace imstk {

//---------------------------------------------------------------------------
Simulator::Simulator()
{
    type = ClassType::Simulator;
    isInitialized = false;
    frameCounter = 0;
    changedMainTimeStamp = 0;
    mainTimeStamp = 0;
    maxThreadCount = 0;
    asyncThreadPoolSize = 0;
    this->name = "Simulator";

    this->assembler = std::make_shared<Assembler>();
    auto interactionConContext = std::make_shared<CollisionContext>();
    this->assembler->setCollisionContext(interactionConContext);
}

/// \brief starts the tasks with the threads from thread pool
//---------------------------------------------------------------------------
void Simulator::beginFrame()
{
    frameCounter++;
}

/// \brief waits until the frame ends
//---------------------------------------------------------------------------
void Simulator::endFrame()
{

}

//---------------------------------------------------------------------------
void Simulator::initAsyncThreadPool()
{
    asyncThreadPoolSize = 0;

    for (size_t i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->getExecutionType() == ObjectSimulator::ExecutionType::AsyncMode)
        {
            asyncThreadPoolSize++;
        }
    }

    asyncPool = make_unique<ThreadPool>(asyncThreadPoolSize);
}

/// \brief the main simulation loop
//---------------------------------------------------------------------------
void Simulator::run()
{
    std::vector< std::future<int> > results;
    std::vector< std::future<int> > asyncResults;
    std::shared_ptr<ObjectSimulator> objectSimulator;

    if (!this->isInitialized)
    {
        // TODO: Log error
        return;
    }

    results.reserve(this->simulators.size()); //make space for results

    //Start up async threads
    asyncResults.reserve(this->asyncThreadPoolSize);
    for (size_t i = 0; i < simulators.size(); i++)
    {
        if (simulators[i]->getExecutionType() == ObjectSimulator::ExecutionType::AsyncMode)
        {
            objectSimulator = simulators[i];
            asyncResults.emplace_back(asyncPool->enqueue(
                [objectSimulator]()
                {
                    objectSimulator->exec();
                    return 0; //this return is just so we have a results value
                })
            );
        }
    }

    while (!this->terminateExecution)
    {
        beginModule();

        results.clear();
        for (size_t i = 0; i < this->simulators.size(); i++)
        {
            objectSimulator = simulators[i];

            if (objectSimulator->getExecutionType() == ObjectSimulator::ExecutionType::AsyncMode)
            {
                continue;
            }

            if (!objectSimulator->isEnabled())
            {
                continue;
            }

            //start each simulator in it's own thread (as max threads allow...)
            results.emplace_back(threadPool->enqueue(
                [objectSimulator,this]()
                {
                    objectSimulator->exec();
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
        results.emplace_back(threadPool->enqueue(
            [this]()
            {
                this->assembler->type1Interactions();
                return 0; //this return is just so we have a results value
            }));

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
//---------------------------------------------------------------------------
void Simulator::registerObjectSimulator(std::shared_ptr<ObjectSimulator> objectSimulator)
{
    simulators.emplace_back(objectSimulator);
    objectSimulator->setEnabled(true);
}

/// \brief
//---------------------------------------------------------------------------
void Simulator::registerCollisionDetection(std::shared_ptr<CollisionDetection> p_collisionDetection)
{
    collisionDetectors.emplace_back(p_collisionDetection);
}

/// \brief
//---------------------------------------------------------------------------
void Simulator::registerContactHandling(std::shared_ptr<ContactHandling> p_contactHandling)
{
    contactHandlers.emplace_back(p_contactHandling);
}

//---------------------------------------------------------------------------
bool Simulator::init()
{
    if(isInitialized)
    {
        return false;
    }
    if(maxThreadCount == 0)
    {
        maxThreadCount = std::max(simulators.size(), collisionDetectors.size());
    }
    threadPool = make_unique<ThreadPool>(maxThreadCount);

    for(size_t i = 0; i < this->simulators.size(); i++)
    {
        simulators[i]->initialize();
    }

    initAsyncThreadPool();
    this->initAssembler();
    isInitialized = true;

    return true;
}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
void Simulator::addCollisionPair(std::shared_ptr< CollisionManager > pair)
{
    collisionPairs.emplace_back(pair);
}
//---------------------------------------------------------------------------
void Simulator::initAssembler()
{
    this->assembler->initSystem();
}

//---------------------------------------------------------------------------
void Simulator::registerInteraction(std::shared_ptr<CollisionManager> pair,
                                    std::shared_ptr<CollisionDetection> collisionDetection,
                                    std::shared_ptr<ContactHandling> contactHandling)
{
    auto interactionConContext = this->assembler->getCollisionContext();

    if(!interactionConContext)
    {
        // TODO: Log this
        return;
    }

    /// TODO: These are legacy functions, will be removed ///
    this->addCollisionPair(pair);
    this->registerCollisionDetection(collisionDetection);
    this->registerContactHandling(contactHandling);
    ////////////////////////////////////////////////////////

    interactionConContext->addInteraction(contactHandling->getFirstInteractionSceneModel(),
                                                contactHandling->getSecondInteractionSceneModel(),
                                                collisionDetection,
                                                contactHandling,
                                                contactHandling,
                                                pair);
}

}
