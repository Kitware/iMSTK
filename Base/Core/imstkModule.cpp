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

#include "imstkModule.h"

#include <iostream>

#include "g3log/g3log.hpp"

namespace imstk {
void
Module::start()
{
    if (m_status != ModuleStatus::INACTIVE)
    {
        LOG(WARNING) << "Can not start '" << m_name << "'.\n"
                     << "Module already/still active.";
        return;
    }

    // Init
    m_status = ModuleStatus::STARTING;
    this->initModule();
    m_status = ModuleStatus::RUNNING;

    // Keep active, wait for terminating call
    while (m_status !=  ModuleStatus::TERMINATING)
    {
        if (m_status == ModuleStatus::PAUSING)
        {
            m_status = ModuleStatus::PAUSED;
        }
        else if (m_status == ModuleStatus::RUNNING)
        {
            this->runModule();
            std::this_thread::sleep_for(std::chrono::milliseconds(m_loopDelay));
        }
    }

    // Cleanup
    this->cleanUpModule();
    m_status = ModuleStatus::INACTIVE;
}

void
Module::run()
{
    if (m_status != ModuleStatus::PAUSED)
    {
        LOG(WARNING) << "Can not run '" << m_name << "'.\n"
                     << "Module not paused.";
        return;
    }

    m_status = ModuleStatus::RUNNING;
}

void
Module::pause()
{
    if (m_status != ModuleStatus::RUNNING)
    {
        LOG(WARNING) << "Can not pause '" << m_name << "'.\n"
                     << "Module not running.";
        return;
    }

    m_status = ModuleStatus::PAUSING;

    while (m_status != ModuleStatus::PAUSED) {}
}

void
Module::end()
{
    if ((m_status == ModuleStatus::INACTIVE) ||
        (m_status == ModuleStatus::TERMINATING))
    {
        LOG(WARNING) << "Can not end '" << m_name << "'.\n"
                     << "Module alreading inactive or terminating.";
        return;
    }

    m_status = ModuleStatus::TERMINATING;

    while (m_status != ModuleStatus::INACTIVE) {}
}

const ModuleStatus&
Module::getStatus() const
{
    return m_status;
}

const std::string&
Module::getName() const
{
    return m_name;
}

const int&
Module::getLoopDelay() const
{
    return m_loopDelay;
}

void
Module::setLoopDelay(int milliseconds)
{
    m_loopDelay = milliseconds;
}
}
