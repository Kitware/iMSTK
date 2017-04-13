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

#include "g3log/g3log.hpp"

namespace imstk
{

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
    if (m_preInitCallback)
    {
        m_preInitCallback(this);
    }
    this->initModule();
    if (m_postInitCallback)
    {
        m_postInitCallback(this);
    }
    m_status = ModuleStatus::RUNNING;

    // Keep active, wait for terminating call
    std::chrono::steady_clock::time_point previous_t = std::chrono::steady_clock::now() - std::chrono::minutes(1);
    std::chrono::steady_clock::time_point current_t;
    int elapsed;
    while (m_status !=  ModuleStatus::TERMINATING)
    {
        if (m_status == ModuleStatus::PAUSING)
        {
            m_status = ModuleStatus::PAUSED;
        }
        else if (m_status == ModuleStatus::RUNNING)
        {
            // Short path to run module if loop delay = 0
            // (updating as fast as possible)
            if(m_loopDelay == 0)
            {
                if (m_preUpdateCallback)
                {
                    m_preUpdateCallback(this);
                }
                this->runModule();
                if (m_postUpdateCallback)
                {
                    m_postUpdateCallback(this);
                }
                continue;
            }

            // If forcing a frequency, wait until enough time elapsed
            current_t = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_t - previous_t).count();
            if(elapsed >= m_loopDelay)
            {
                if (m_preUpdateCallback)
                {
                    m_preUpdateCallback(this);
                }
                this->runModule();
                if (m_postUpdateCallback)
                {
                    m_postUpdateCallback(this);
                }
                previous_t = current_t;
            }
        }
    }

    // Cleanup
    if (m_preCleanUpCallback)
    {
        m_preCleanUpCallback(this);
    }
    this->cleanUpModule();
    if (m_postCleanUpCallback)
    {
        m_postCleanUpCallback(this);
    }
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
                     << "Module already inactive or terminating.";
        return;
    }

    m_status = ModuleStatus::TERMINATING;

    while (m_status != ModuleStatus::INACTIVE) {}
}

const ModuleStatus
Module::getStatus() const
{
    return m_status.load();
}

const std::string&
Module::getName() const
{
    return m_name;
}

double Module::getLoopDelay() const
{
    return m_loopDelay;
}

void
Module::setLoopDelay(const double milliseconds)
{
    if(milliseconds < 0)
    {
        LOG(WARNING) << "Module::setLoopDelay error: delay must be positive.";
        return;
    }
    m_loopDelay = milliseconds;
}

double Module::getFrequency() const
{
    if(m_loopDelay == 0)
    {
        LOG(WARNING) << "Module::getFrequency warning: loop delay is set to 0ms, "
                     << "therefore not regulated by a frequency. Returning 0.";
        return 0;
    }
    return 1000.0/m_loopDelay;
}

void
Module::setFrequency(const double f)
{
    if(f < 0)
    {
        LOG(WARNING) << "Module::setFrequency error: f must be positive, "
                     << "or equal to 0 to run the module in a closed loop.";
        return;
    }
    if(f == 0)
    {
        m_loopDelay = 0;
        return;
    }
    m_loopDelay = 1000.0/f;
}

}
