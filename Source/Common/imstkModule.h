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
    /// \brief Get whether the module is currently paused
    ///
    bool getPaused() const { return m_paused; }

    ExecutionType getExecutionType() const { return m_executionType; }
    void setExecutionType(ExecutionType type) { m_executionType = type; }

    void pause() { m_paused = true; }
    void resume() { m_paused = false; }

public:
    void init() { m_init = initModule(); }

    void update()
    {
        if (m_init && !m_paused)
        {
            this->postEvent(Event(EventType::PreUpdate));
            this->updateModule();
            this->postEvent(Event(EventType::PostUpdate));
        }
    }

    void uninit()
    {
        if (!m_init)
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
    bool   m_init   = false;
    bool   m_paused = false;
    double m_dt     = 0.0;
    ExecutionType m_executionType = ExecutionType::PARALLEL;     // Defaults to parallel, subclass and set
};
}