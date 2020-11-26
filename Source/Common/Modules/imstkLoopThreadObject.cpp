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

#include "imstkLoopThreadObject.h"
#include "imstkLogger.h"
#include "imstkMath.h"
#include "imstkTimer.h"

namespace imstk
{
LoopThreadObject::LoopThreadObject(std::string name, int loopDelay) :
    ThreadObject(name),
    m_frameCounter(std::make_shared<UPSCounter>()),
    m_loopDelay(loopDelay),
    m_trackFPS(false),
    m_isInitialized(false)
{
}

void
LoopThreadObject::startThread()
{
    init();

    // Keep active, wait for terminating call
    std::chrono::steady_clock::time_point previous_t = std::chrono::steady_clock::now() - std::chrono::minutes(1);
    std::chrono::steady_clock::time_point current_t;
    long long                             elapsed;
    while (m_status != ThreadStatus::Inactive)
    {
        if (m_requestedStatus == ThreadStatus::Inactive)
        {
            m_status = ThreadStatus::Inactive;
        }
        else if (m_requestedStatus == ThreadStatus::Paused)
        {
            m_status = ThreadStatus::Paused;
        }
        else if (m_requestedStatus == ThreadStatus::Running)
        {
            m_status = ThreadStatus::Running;
        }

        if (m_status == ThreadStatus::Running)
        {
            // Short path to run module if loop delay = 0
            // (updating as fast as possible)
            if (m_loopDelay < VERY_SMALL_EPSILON_D)
            {
                if (m_trackFPS)
                {
                    m_frameCounter->setStartPointOfUpdate();
                }

                update();

                if (m_trackFPS)
                {
                    m_frameCounter->setEndPointOfUpdate();

                    std::cout << "\r" << this->getName() << " running at "
                              << m_frameCounter->getUPS() << " ups   " << std::flush;
                }
            }
            else
            {
                // If forcing a frequency, wait until enough time elapsed
                current_t = std::chrono::steady_clock::now();
                elapsed   = std::chrono::duration_cast<std::chrono::milliseconds>(current_t - previous_t).count();
                if (elapsed >= m_loopDelay)
                {
                    if (m_trackFPS)
                    {
                        m_frameCounter->setStartPointOfUpdate();
                    }

                    update();

                    if (m_trackFPS)
                    {
                        m_frameCounter->setEndPointOfUpdate();
                    }

                    previous_t = current_t;
                }
            }
        }
    }
}

void
LoopThreadObject::setLoopDelay(const double milliseconds)
{
    if (milliseconds < 0)
    {
        LOG(WARNING) << "Module::setLoopDelay error: delay must be positive.";
        return;
    }
    m_loopDelay = milliseconds;
}

double
LoopThreadObject::getFrequency() const
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
LoopThreadObject::setFrequency(const double f)
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
LoopThreadObject::getUPS() const
{
    if (m_status != ThreadStatus::Running)
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