/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
class ModuleDriver : public EventObject, public std::enable_shared_from_this<ModuleDriver>
{
protected:
    ModuleDriver() = default;
public:
    ~ModuleDriver() override = default;

public:
    virtual void start() = 0;

    ///
    /// \brief Add a module to run
    ///
    virtual void addModule(std::shared_ptr<Module> module)
    {
        m_modules.push_back(module);
    }

    ///
    /// \brief Remove all modules
    ///
    virtual void clearModules() { m_modules.clear(); }

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