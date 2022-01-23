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

#include "imstkEventObject.h"

namespace imstk
{
using ModuleDriverStatus = int;
#define ModuleDriverRunning 0
#define ModuleDriverPaused 1
#define ModuleDriverStopped 2

class Module;

///
/// \class ModuleDriver
///
/// \brief Defines the control of modules
///
class ModuleDriver : public EventObject
{
protected:
    ModuleDriver() = default;
public:
    virtual ~ModuleDriver() override = default;

public:
    virtual void start() = 0;

    ///
    /// \brief Add a module to run
    ///
    virtual void addModule(std::shared_ptr<Module> module)
    {
        m_modules.push_back(module);
    }

    void requestStatus(ModuleDriverStatus status) { simState = status; }
    ModuleDriverStatus getStatus() const { return simState; }

    std::vector<std::shared_ptr<Module>>& getModules() { return m_modules; }

    ///
    /// \brief Wait for all modules to init
    ///
    void waitForInit();

protected:
    std::vector<std::shared_ptr<Module>> m_modules;

    std::atomic<ModuleDriverStatus> simState = { ModuleDriverRunning };
};
}; // namespace imstk