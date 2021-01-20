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

#include "imstkSubstepModuleDriver.h"
#include "imstkViewer.h"
#include "imstkTimer.h"

#include <thread>
#include <tbb/task.h>

class FuncTask : public tbb::task
{
public:
    FuncTask(std::shared_ptr<imstk::Module> module, std::function<void(std::shared_ptr<imstk::Module>)> func) :
        m_func(func), m_module(module)
    {
    }

    task* execute() override
    {
        __TBB_ASSERT(ref_count() == 0, NULL);

        m_func(m_module);

        return NULL;
    }

protected:
    std::function<void(std::shared_ptr<imstk::Module>)> m_func;
    std::shared_ptr<imstk::Module> m_module;
};

namespace imstk
{
void
SubstepModuleDriver::start()
{
    // Modules can cause a full exit internally
    // particularly needed for viewers which contain OS event loop, so when the window
    // exit message happens all modules need to stop
    for (auto module : m_modules)
    {
        connect<Event>(module, EventType::End, this, &SubstepModuleDriver::requestStop);
    }

    // Initialization
    for (auto viewer : m_viewers)
    {
        viewer->init();
    }
    for (auto syncModule : m_syncModules)
    {
        syncModule->init();
    }
    for (auto adaptiveModule : m_adaptiveModules)
    {
        adaptiveModule->init();
    }

    // Start parallel modules
    tbb::task_list           taskList;
    std::vector<std::thread> threads(m_asyncModules.size());
    {
        if (m_threadType == ThreadingType::TBB)
        {
            for (auto module : m_asyncModules)
            {
                FuncTask* moduleTask = new(tbb::task::allocate_root())FuncTask(module,
                    std::bind(&SubstepModuleDriver::runModuleParallel, this, std::placeholders::_1));
                taskList.push_back(*moduleTask);
            }
            tbb::task::spawn_root_and_wait(taskList);
        }
        else if (m_threadType == ThreadingType::STL)
        {
            for (size_t i = 0; i < m_asyncModules.size(); i++)
            {
                std::shared_ptr<Module> module = m_asyncModules[i];
                threads[i] = std::thread(std::bind(&SubstepModuleDriver::runModuleParallel, this, module));
            }
        }
    }

    waitForInit();

    // Start the game loop
    {
        const double desiredDt_ms = m_desiredDt * 1000.0; // ms
        m_numSteps = 0;
        double    accumulator = 0.0;
        StopWatch timer;
        timer.start();
        bool running = true;

        // Mark all as running but async modules
        for (auto module : m_viewers)
        {
            m_running[module.get()] = true;
        }
        for (auto module : m_syncModules)
        {
            m_running[module.get()] = true;
        }
        for (auto module : m_adaptiveModules)
        {
            m_running[module.get()] = true;
        }

        while (running)
        {
            const int newState = simState;
            if (newState == ModuleDriverStopped)
            {
                running = false;
                continue;
            }

            const double passedTime = timer.getTimeElapsed();
            timer.start();

            if (newState == ModuleDriverPaused)
            {
                continue;
            }

            // Accumulate the real time passed
            accumulator += passedTime;

            // Compute number of steps we can take (total time previously took / desired time step)
            m_numSteps = 0;
            while (accumulator >= desiredDt_ms)
            {
                m_numSteps++;
                accumulator -= desiredDt_ms;
            }

            // Flatten out the leftover accumulation in our chosen dt
            m_dt = desiredDt_ms;
            if (m_numSteps != 0)
            {
                m_dt += accumulator / m_numSteps;
                accumulator = 0.0;
            }
            m_dt *= 0.001; // ms->s

            // Optional smoothening + loss here

            // Actual game loop
            {
                for (auto viewer : m_viewers)
                {
                    viewer->processEvents();
                }

                for (auto syncModule : m_syncModules)
                {
                    syncModule->setDt(m_dt);
                    syncModule->update();
                }

                for (auto adaptiveModule : m_adaptiveModules)
                {
                    adaptiveModule->setDt(m_dt);
                    for (int currStep = 0; currStep < m_numSteps; currStep++)
                    {
                        adaptiveModule->update();
                    }
                }

                for (auto viewer : m_viewers)
                {
                    viewer->update();
                }
            }
        }
    }

    if (m_threadType == ThreadingType::TBB)
    {
    }
    else if (m_threadType == ThreadingType::STL)
    {
        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }
    }

    for (auto module : m_modules)
    {
        m_running[module.get()] = false;
        module->uninit();
    }
}

void
SubstepModuleDriver::addModule(std::shared_ptr<Module> module)
{
    ModuleDriver::addModule(module);

    if (std::shared_ptr<Viewer> viewer = std::dynamic_pointer_cast<Viewer>(module))
    {
        m_viewers.push_back(viewer);
        return;
    }

    if (module->getExecutionType() == Module::ExecutionType::SEQUENTIAL)
    {
        m_syncModules.push_back(module);
    }
    else if (module->getExecutionType() == Module::ExecutionType::PARALLEL)
    {
        m_asyncModules.push_back(module);
    }
    else if (module->getExecutionType() == Module::ExecutionType::ADAPTIVE)
    {
        m_adaptiveModules.push_back(module);
    }
}

void
SubstepModuleDriver::runModuleParallel(std::shared_ptr<Module> module)
{
    module->init();

    waitForInit();

    m_running[module.get()] = true;
    while (m_running[module.get()])
    {
        // ModuleDriver state will stop/pause/run all modules
        const int newState = simState;
        if (newState == ModuleDriverStopped)
        {
            m_running[module.get()] = false;
        }
        else if (newState == ModuleDriverRunning)
        {
            std::shared_ptr<Viewer> viewer = std::dynamic_pointer_cast<Viewer>(module);
            if (viewer != nullptr)
            {
                viewer->processEvents();
            }

            module->update();
        }
    }
}

void
SubstepModuleDriver::requestStop(Event* e)
{
    Module* module = static_cast<Module*>(e->m_sender);
    if (module != nullptr)
    {
        requestStatus(ModuleDriverStopped);
        m_running[module] = false;
    }
}
}