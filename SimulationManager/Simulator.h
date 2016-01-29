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

#ifndef SIMULATION_MANAGER_SIMULATOR_H
#define SIMULATION_MANAGER_SIMULATOR_H

// STL include
#include <memory>

// iMSTK includes
#include "Core/Module.h"

class ThreadPool;

namespace imstk {

class ObjectSimulator;
class CollisionDetection;
class ContactHandling;
class Assembler;
class CollisionManager;
class CollisionContext;

class Simulator: public Module
{
private:
    /// \brief Initializes up asynchronous threadpool
    void initAsyncThreadPool();

public:
    ///initializes all the simulators in the objects in the scene..
    bool init();

    /// \brief constructor gets error log
    Simulator();

    void setMaxThreadCount(int p_threadMaxCount);

    ///Simualtor registers the simulator and schedules it.
    ///the function is reentrant it is not thread safe.
    void registerObjectSimulator(std::shared_ptr<ObjectSimulator> objectSimulator);

    void registerCollisionDetection(std::shared_ptr<CollisionDetection> p_collisionDetection);

    void registerContactHandling(std::shared_ptr<ContactHandling> p_contactHandling);

    void addCollisionPair(std::shared_ptr<CollisionManager> pair);

    void registerInteraction(
        std::shared_ptr<CollisionManager> pair,
        std::shared_ptr<CollisionDetection> p_collisionDetection,
        std::shared_ptr<ContactHandling> p_contactHandling);

    /// \brief the actual implementation of the simulator module resides in run function
    void run();

    /// \brief called at the beginning of  each and every frame
    virtual void beginFrame();

    /// \brief called at the end of each and every frame
    virtual void endFrame();

    /// \brief this is called by SDK. it lanuches the simulator module
    virtual void exec();

    ///
    /// \brief Initializes the assembler with interaction context
    ///
    void initAssembler();

private:
    std::vector<std::shared_ptr<ObjectSimulator>> simulators;
    std::vector<std::shared_ptr<CollisionDetection>> collisionDetectors;
    std::vector<std::shared_ptr<CollisionManager>> collisionPairs;
    std::vector<std::shared_ptr<ContactHandling>> contactHandlers;

    std::shared_ptr<Assembler> assembler; // Main interaction objects assembler

    std::unique_ptr<ThreadPool> threadPool; //
    std::unique_ptr<ThreadPool> asyncPool; // asynchronous thread pool

    unsigned int frameCounter; // module keeps track of frame number
    int maxThreadCount; // maximum number of threads
    int changedMainTimeStamp;
    int mainTimeStamp; // time stamp when main callback is registered
    int asyncThreadPoolSize; // Tracks the number of threads the async threadpool is running
};

}

#endif
