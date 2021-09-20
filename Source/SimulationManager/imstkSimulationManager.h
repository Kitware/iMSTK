/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#pragma once

#include "imstkModuleDriver.h"

#include <unordered_map>

namespace imstk
{
class Viewer;

///
/// \class SimulationManager
///
/// \brief Defines a sequential substepping approach to driving the modules
/// and rendering. The user provides a desired timestep and as it runs it
/// accumulates time. It then determines how many simulation steps before
/// every render (simply accumulated time / timestep = substeps). The remainder
/// is divided out over the substeps.
/// This is the preferred driver.
/// todo: Timestep smoothening
///
/// Events: Posts `EventType::Start` just before the beginning of the loop,
/// posts `EventType::Stop` just after the processing loops is being exited
class SimulationManager : public ModuleDriver
{
public:
    // Generally STL is better for less interruption
    enum class ThreadingType
    {
        TBB,
        STL
    };

public:

    SimulationManager() = default;
    virtual ~SimulationManager() override = default;

public:
    // *INDENT-OFF*
    ///
    /// \brief Called after initialization but before starting the loop
    ///
    SIGNAL(SimulationManager, starting);

    ///
    /// \brief Called after ending, but before un initialization
    ///
    SIGNAL(SimulationManager, ending);
    // *INDENT-ON*

public:
    void start() override;

    ///
    /// \brief Add a module to run
    /// \param The module to run
    ///
    void addModule(std::shared_ptr<Module> module);

    ///
    /// \brief Sets the target fixed timestep (may violate), seconds
    /// This ultimately effects the number of iterations done
    /// default 0.003
    ///
    void setDesiredDt(const double dt)
    {
        m_desiredDt = dt;
    }

    double getDesiredDt() const
    {
        return m_desiredDt;
    }

    ///
    /// \brief Get the current actual timestep
    ///
    double getDt() const
    {
        return m_dt;
    }

    ///
    /// \brief Set the thread type to run the parallel modules with
    ///
    void setThreadType(ThreadingType threadType)
    {
        m_threadType = threadType;
    }

protected:
    void requestStop(Event* e);

    void runModuleParallel(std::shared_ptr<Module> module);

protected:
    std::vector<std::shared_ptr<Viewer>> m_viewers;

    std::unordered_map<Module*, bool> m_running;

    std::vector<std::shared_ptr<Module>> m_syncModules;      ///> Modules called once per update
    std::vector<std::shared_ptr<Module>> m_asyncModules;     ///> Modules that run on completely other threads without restraint
    std::vector<std::shared_ptr<Module>> m_adaptiveModules;  ///> Modules that update adpatively to keep up with real time

    ThreadingType m_threadType = ThreadingType::STL;
    double m_desiredDt = 0.003; // Desired timestep
    double m_dt       = 0.0;    // Actual timestep
    int    m_numSteps = 0;
};
};
