/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTaskNode.h"
#include "imstkTimer.h"

namespace imstk
{
std::atomic<size_t> TaskNode::s_numGlobalIds = { 0 };

void
TaskNode::execute()
{
    if (m_enabled && m_func != nullptr)
    {
        if (!m_enableTiming)
        {
            m_func();
        }
        else
        {
            StopWatch timer;
            timer.start();
            m_func();
            m_computeTime = timer.getTimeElapsed();
            timer.stop();
        }
    }
    else
    {
        m_computeTime = 0.0;
    }
}
} // namespace imstk