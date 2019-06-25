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

#include <iostream>
#include <atomic>
#include <functional>

namespace imstk
{
///
/// \brief Enumerations for the status of the module
///
enum class ModuleStatus
{
    STARTING,
    RUNNING,
    PAUSING,
    PAUSED,
    TERMINATING,
    INACTIVE
};

///
/// \class Module
///
/// \brief iMSTK module
///
class Module
{
using CallbackFunction = std::function<void (Module* module)>;

public:
    ///
    /// \brief Constructor
    ///
    Module(std::string name, int loopDelay = 0) :
        m_name(name),
        m_loopDelay(loopDelay)
    {}

    ///
    /// \brief Destructor
    ///
    virtual ~Module() = default;

    ///
    /// \brief Start the module
    ///
    void start();

    ///
    /// \brief Run module
    ///
    void run();

    ///
    /// \brief Pause the module
    ///
    void pause();

    ///
    /// \brief End the module
    ///
    void end();

    inline void setPreInitCallback(CallbackFunction foo) { m_preInitCallback = foo; }
    inline void setPostInitCallback(CallbackFunction foo) { m_postInitCallback = foo; }
    inline void setPreUpdateCallback(CallbackFunction foo) { m_preUpdateCallback = foo; }
    inline void setPostUpdateCallback(CallbackFunction foo) { m_postUpdateCallback = foo; }
    inline void setPreCleanUpCallback(CallbackFunction foo) { m_preCleanUpCallback = foo; }
    inline void setPostCleanUpCallback(CallbackFunction foo) { m_postCleanUpCallback = foo; }

    ///
    /// \brief Get the status of the module
    ///
    ModuleStatus getStatus() const;

    ///
    /// \brief Get the name of the module
    ///
    const std::string& getName() const;

    ///
    /// \brief Get loop delay
    ///
    double getLoopDelay() const;

    ///
    /// \brief Set the loop delay
    ///
    void setLoopDelay(const double milliseconds);

    ///
    /// \brief Get loop delay
    ///
    double getFrequency() const;

    ///
    /// \brief Set the loop delay
    ///
    void setFrequency(const double f);

protected:

    ///
    /// \brief Initialize module
    ///
    virtual void initModule() = 0;

    ///
    /// \brief Run the module
    ///
    virtual void runModule() = 0;

    ///
    /// \brief Clean the module
    ///
    virtual void cleanUpModule() = 0;

    CallbackFunction m_preInitCallback;     ///> function callback preceding module initialization
    CallbackFunction m_postInitCallback;    ///> function callback following module initialization
    CallbackFunction m_preUpdateCallback;   ///> function callback preceding module update
    CallbackFunction m_postUpdateCallback;  ///> function callback following module update
    CallbackFunction m_preCleanUpCallback;  ///> function callback preceding module cleanup
    CallbackFunction m_postCleanUpCallback; ///> function callback following module cleanup

    std::atomic<ModuleStatus> m_status {ModuleStatus::INACTIVE}; ///> Module status

    std::string m_name;     ///> Name of the module
    double m_loopDelay = 0; ///> Loop delay
};
}
