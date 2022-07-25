/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkModule.h"
#include "imstkLogger.h"

#include <thread>

namespace imstk
{
void
Module::setSleepDelay(const double ms)
{
    CHECK(ms >= 0.0);
    m_sleepDelay = ms;
}

void
Module::update()
{
    if (m_init && !m_paused)
    {
        if (m_sleepDelay != 0.0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(m_sleepDelay));
        }

        if (m_muteUpdateEvents)
        {
            this->updateModule();
        }
        else
        {
            this->postEvent(Event(Module::preUpdate()));
            this->updateModule();
            this->postEvent(Event(Module::postUpdate()));
        }
    }
}

void
Module::uninit()
{
    // Can only uninit if, init'd
    if (m_init)
    {
        uninitModule();
        m_init = false;
    }
}
}// imstk
