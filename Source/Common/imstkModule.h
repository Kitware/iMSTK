///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#pragma once

#include "imstkTimer.h"
#include "imstkEventObject.h"

#include <tbb/atomic.h>
#include <thread>

namespace imstk
{
///
/// \class Module
///
/// \brief Base class for imstk module system. A module defines something
/// that is updated, and can be paused/resumed. *It cannot start or stop*
/// Update loops are defined externally by a driver so they may be replaced.
///
class Module : public EventObject
{
public:
    ///
    /// \brief Currently 3 execution types are provided.
    /// These inform the driver on how it should run
    ///
    enum class ExecutionType
    {
        SEQUENTIAL,      // Should run in sync with other sequential modules
        PARALLEL,        // Should run in parallel
        ADAPTIVE         // Runs governed by module
    };

public:
    Module() = default;
    virtual ~Module() override = default;

public:
    // *INDENT-OFF*
    ///
    /// \brief Posted before updateModule is called
    ///
    SIGNAL(Module, preUpdate);

    ///
    /// \brief Posted after updateModule is called
    ///
    SIGNAL(Module, postUpdate);

    ///
    /// \brief Posted when module wants to end
    ///
    SIGNAL(Module, end);
    // *INDENT-ON*

public:
    ///
    /// \brief Get/Set the timestep
    ///
    double getDt() const { return m_dt; }
    void setDt(const double dt) { m_dt = dt; }

    ///
    /// \brief Get whether the module is initialized yet
    ///
    bool getInit() const { return m_init; }

    ///
    /// \brief Set/Get whether the module is currently paused
    ///
    bool getPaused() const { return m_paused; }
    void setPaused(const bool paused) { m_paused = paused; }

    ExecutionType getExecutionType() const { return m_executionType; }
    void setExecutionType(const ExecutionType type) { m_executionType = type; }

    void setSleepDelay(const double ms) { sleepDelay = ms; }

    void pause() { m_paused = true; }
    void resume() { m_paused = false; }

public:
    void init() { m_init = initModule(); }

    void update()
    {
        if (m_init && !m_paused)
        {
            if (sleepDelay != 0.0)
            {
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepDelay));
            }

            if (muteUpdateEvents)
            {
                this->updateModule();
            }
            else
            {
                this->postEvent(Event(Module::preUpdate()));
                this->updateModule();
                this->postEvent(Event(Module::postUpdate()));
            }
        }
    }

    void uninit()
    {
        // Can only uninit if, init'd
        if (m_init)
        {
            uninitModule();
            m_init = false;
        }
    }

public:
    virtual bool initModule() = 0;

    virtual void updateModule() = 0;

    virtual void uninitModule() { }

protected:
    tbb::atomic<bool> m_init   = false;
    tbb::atomic<bool> m_paused = false;
    double m_dt = 0.0;
    ExecutionType m_executionType = ExecutionType::PARALLEL; // Defaults to parallel, subclass and set
    bool   muteUpdateEvents       = false;                   // Avoid posting pre/post update, useful when running modules at extremely fast rates
    double sleepDelay = 0.0;                                 // ms sleep for the module, useful for throttling some modules
};
}