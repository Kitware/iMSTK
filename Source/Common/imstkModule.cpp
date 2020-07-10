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
#include "imstkLogger.h"
#include "imstkMath.h"
#include "imstkTimer.h"

namespace imstk
{
Module::Module(const std::string& name, int loopDelay /*= 0*/) :
    m_name(name),
    m_frameCounter(std::make_shared<UPSCounter>()),
    m_trackFPS(false),
    m_loopDelay(loopDelay)
{
}

void
Module::start()
{
    if (m_status != ModuleStatus::Inactive)
    {
        LOG(WARNING) << "Can not start '" << m_name << "'.\n"
                     << "Module already/still active.";
        return;
    }

    // Init
    m_status = ModuleStatus::Starting;
    if (m_preInitCallback)
    {
        m_preInitCallback(this);
    }

    this->initModule();

    if (m_postInitCallback)
    {
        m_postInitCallback(this);
    }

    m_status = ModuleStatus::Running;

    // Keep active, wait for terminating call
    std::chrono::steady_clock::time_point previous_t = std::chrono::steady_clock::now() - std::chrono::minutes(1);
    std::chrono::steady_clock::time_point current_t;
    long long                             elapsed;
    while (m_status != ModuleStatus::Terminating)
    {
        if (m_status == ModuleStatus::Pausing)
        {
            m_status = ModuleStatus::Paused;
        }
        else if (m_status == ModuleStatus::Running)
        {
            // Short path to run module if loop delay = 0
            // (updating as fast as possible)
            if (m_loopDelay < VERY_SMALL_EPSILON_D)
            {
                if (m_trackFPS)
                {
                    m_frameCounter->setStartPointOfUpdate();
                }

                if (m_preUpdateCallback)
                {
                    m_preUpdateCallback(this);
                }
                this->runModule();
                if (m_postUpdateCallback)
                {
                    m_postUpdateCallback(this);
                }

                if (m_trackFPS)
                {
                    m_frameCounter->setEndPointOfUpdate();

                    std::cout << "\r" << this->getName() << " running at "
                              << m_frameCounter->getUPS() << " ups   " << std::flush;
                }
                continue;
            }

            // If forcing a frequency, wait until enough time elapsed
            current_t = std::chrono::steady_clock::now();
            elapsed   = std::chrono::duration_cast<std::chrono::milliseconds>(current_t - previous_t).count();
            if (elapsed >= m_loopDelay)
            {
                if (m_trackFPS)
                {
                    m_frameCounter->setStartPointOfUpdate();
                }

                if (m_preUpdateCallback)
                {
                    m_preUpdateCallback(this);
                }
                this->runModule();
                if (m_postUpdateCallback)
                {
                    m_postUpdateCallback(this);
                }

                if (m_trackFPS)
                {
                    m_frameCounter->setEndPointOfUpdate();
                }

                previous_t = current_t;
            }
        }
    }

    m_status = ModuleStatus::Inactive;
}

void
Module::run()
{
    if (m_status != ModuleStatus::Paused)
    {
        return;
    }

    m_status = ModuleStatus::Running;
}

void
Module::pause()
{
    if (m_status == ModuleStatus::Running)
    {
        m_status = ModuleStatus::Pausing;

        while (m_status != ModuleStatus::Paused) {}
    }
}

void
Module::end()
{
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

    if ((m_status == ModuleStatus::Inactive)
        || (m_status == ModuleStatus::Terminating))
    {
        LOG(WARNING) << "Can not end '" << m_name << "'.\n"
                     << "Module already inactive or terminating.";
        return;
    }

    m_status = ModuleStatus::Terminating;

    while (m_status != ModuleStatus::Inactive) {}
}

ModuleStatus
Module::getStatus() const
{
    return m_status.load();
}

const std::string&
Module::getName() const
{
    return m_name;
}

double
Module::getLoopDelay() const
{
    return m_loopDelay;
}

void
Module::setLoopDelay(const double milliseconds)
{
    if (milliseconds < 0)
    {
        LOG(WARNING) << "Module::setLoopDelay error: delay must be positive.";
        return;
    }
    m_loopDelay = milliseconds;
}

double
Module::getFrequency() const
{
    if (m_loopDelay < VERY_SMALL_EPSILON_D)
    {
        LOG(WARNING) << "Module::getFrequency warning: loop delay is set to 0ms, "
                     << "therefore not regulated by a frequency. Returning 0.";
        return 0;
    }
    return 1000.0 / m_loopDelay;
}

void
Module::setFrequency(const double f)
{
    if (f < 0)
    {
        LOG(WARNING) << "Module::setFrequency error: f must be positive, "
                     << "or equal to 0 to run the module in a closed loop.";
        return;
    }
    if (f < VERY_SMALL_EPSILON_D)
    {
        m_loopDelay = 0;
        return;
    }
    m_loopDelay = 1000.0 / f;
}

unsigned int
Module::getUPS()
{
    if (m_status != ModuleStatus::Running)
    {
        return 0;
    }

    if (m_trackFPS)
    {
        return m_frameCounter->getUPS();
    }
    else
    {
        LOG(WARNING) << "Frame counter is not enabled!";
        return 0;
    }
}
}
