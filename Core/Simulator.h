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

#ifndef SMSIMULATOR_H
#define SMSIMULATOR_H

// STL include
#include <memory>

// Threads includes
#include <ThreadPool.h>

// SimMedTK includes
#include "Core/Module.h"
#include "Core/ObjectSimulator.h"
#include "Core/MakeUnique.h"
#include "Core/CollisionDetection.h"
#include "Core/ContactHandling.h"

struct SimulationMainParam
{
    std::vector<std::shared_ptr<Scene>> sceneList;
};

/// \brief call back for simulator module. simulateMain is called in every simulation module frame.
class SimulationMain
{
public:
    virtual void simulateMain(const SimulationMainParam &) = 0;
};

class Simulator: public Module
{
    friend class SDK;

private:
    /// \brief Initializes up asynchronous threadpool
    void initAsyncThreadPool();

public:
    ///initializes all the simulators in the objects in the scene..
    void init();

    /// \brief constructor gets error log
    Simulator(std::shared_ptr<ErrorLog> p_log);

    void setMaxThreadCount(int p_threadMaxCount);

    ///Simualtor registers the simulator and schedules it.
    ///the function is reentrant it is not thread safe.
    void registerObjectSimulator(std::shared_ptr<ObjectSimulator> objectSimulator);

    void registerCollisionDetection(std::shared_ptr<CollisionDetection> p_collisionDetection);

    void registerContactHandling(std::shared_ptr<ContactHandling> p_contactHandling);

    ///Registration of the Simulation main. It is called in each and every frame
    void registerSimulationMain(std::shared_ptr<SimulationMain> p_main);

    /// \brief the actual implementation of the simulator module resides in run function
    void run();

    /// \brief called at the beginning of  each and every frame
    virtual void beginFrame();

    /// \brief called at the end of each and every frame
    virtual void endFrame();

    /// \brief this is called by SDK. it lanuches the simulator module
    virtual void exec();

    void addCollisionPair(std::shared_ptr<CollisionPair> pair)
    {
        collisionPairs.emplace_back(pair);
    }

private:
    std::vector<std::shared_ptr<ObjectSimulator>> simulators;
    std::vector<std::shared_ptr<CollisionDetection>> collisionDetectors;
    std::vector<std::shared_ptr<CollisionPair>> collisionPairs;
    std::vector<std::shared_ptr<ContactHandling>> contactHandlers;

    std::unique_ptr<ThreadPool> threadPool; //
    std::unique_ptr<ThreadPool> asyncPool; // asynchronous thread pool
    std::shared_ptr<ErrorLog> log; // error log
    std::shared_ptr<SimulationMain> main; // Simulation main registration
    std::shared_ptr<SimulationMain> changedMain; // for updating the main in real-time. The change has effect after a frame is completed

    unsigned int frameCounter; // module keeps track of frame number
    int maxThreadCount; // maximum number of threads
    int changedMainTimeStamp;
    int mainTimeStamp; // time stamp when main callback is registered
    int asyncThreadPoolSize; // Tracks the number of threads the async threadpool is running
};

#endif
