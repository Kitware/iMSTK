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

#ifndef imstkModule_h
#define imstkModule_h

#include <iostream>
#include <atomic>

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
    ~Module() = default;

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

    ///
    /// \brief Get the status of the module
    ///
    const ModuleStatus getStatus() const;

    ///
    /// \brief Get the name of the module
    ///
    const std::string& getName() const;

    ///
    /// \brief Get loop delay
    ///
    const int& getLoopDelay() const;

    ///
    /// \brief Set the loop delay
    ///
    void setLoopDelay(int milliseconds);

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

    std::atomic<ModuleStatus> m_status{ModuleStatus::INACTIVE};///> Module status

    std::string  m_name;    ///> Name of the module
    int m_loopDelay;        ///> Loop delay
};

}

#endif // ifndef imstkModule_h
