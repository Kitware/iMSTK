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

#ifndef SIMULATORS_OBJECTSIMULATOR_H
#define SIMULATORS_OBJECTSIMULATOR_H

// iMSTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/Timer.h"
#include "SimulationManager/Scheduler.h"

namespace imstk {

// Forward declarations
class SceneObject;

///
/// \brief This is the major object simulator. Each object simulator should derive this class.
///     you want particular object simulator to work over an object just set pointer of the
///     object. The rest will be taken care of the simulator and object simulator.
///
class ObjectSimulator : public CoreClass
{
public:
    enum class ExecutionType
    {
        SyncMode,
        AsyncMode
    };

public:
    ///
    /// \brief Constructor
    ///
    ObjectSimulator();

    ///
    /// \brief Add a scene model to the list.
    ///     The function is re-entrant it is not thread safe.
    ///
    /// \param model Scene model.
    ///
    virtual void addModel(std::shared_ptr<SceneObject> model);

    ///
    /// \brief remove object from the simulator
    ///
    virtual void removeModel(const std::shared_ptr<SceneObject> &model);

    ///
    /// \brief Set the ExecutionType for this simulator.
    ///
    void setExecutionType(const ExecutionType &type);

    ///
    /// \brief Get the ExecutionType of this simulator.
    ///
    ExecutionType getExecutionType() const;

    ///
    /// \brief Return the enabled variable.
    ///
    bool isEnabled();

    ///
    /// \brief Return the enabled variable.
    ///
    void setEnabled(bool value);

    ///
    /// \brief Initialize simulator.
    ///
    virtual void initialize();

    ///
    /// \brief synchronization
    ///
    virtual void syncBuffers();

    ///
    /// \brief This is the actual work function for this simulator.
    ///
    void exec();

    ///
    /// \brief Set TimeStep
    ///
    void setTimeStep(const double newTimeStep);

    ///
    /// \brief Get TimeStep
    ///
    double getTimeStep() const;

    ///
    /// \brief Execute the simulation.
    ///
    virtual void run();

protected:
    ///
    /// \brief  Called at the beginning of the simulator frame.
    ///
    virtual void beginExecution();

    ///
    /// \brief Called at the end of simulation frame.
    ///
    virtual void endExecution();

protected:
    bool enabled; ///> Set to enable/disable this simulator.
    bool isObjectSimInitialized; ///> Initialization flag.
    double timeStep;
    long double timerPerFrame; ///> Time spent on each frame.
    long double framesPerSecond; ///>
    unsigned int frameCounter;
    long double totalTime; ///> Total accumulated time.
    bool executionTypeStatusChanged; ///> Set in order to indicate that this simulator changed its execution type.
    ExecutionType executionType; ///> Either synchronous or asynchronous execution for this simulator.
    Timer timer;          ///> Timer.
    std::vector<std::shared_ptr<SceneObject>> simulatedModels; ///> List of simulated models to run.
};

}

#endif
