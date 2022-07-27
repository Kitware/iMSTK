/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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

    SimulationManager() = default;
    ~SimulationManager() override = default;

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

    void start() override;

    ///
    /// \brief Add a module to run
    /// \param The module to run
    ///
    void addModule(std::shared_ptr<Module> module) override;

    ///
    /// \brief Remove all modules
    ///
    void clearModules() override;

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

    std::vector<std::shared_ptr<Viewer>> m_viewers;

    std::unordered_map<Module*, bool> m_running;

    std::vector<std::shared_ptr<Module>> m_syncModules;      ///< Modules called once per update
    std::vector<std::shared_ptr<Module>> m_asyncModules;     ///< Modules that run on completely other threads without restraint
    std::vector<std::shared_ptr<Module>> m_adaptiveModules;  ///< Modules that update adpatively to keep up with real time

    ThreadingType m_threadType = ThreadingType::STL;
    double m_desiredDt = 0.003; // Desired timestep
    double m_dt       = 0.0;    // Actual timestep
    int    m_numSteps = 0;
};
}; // namespace imstk
