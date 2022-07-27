/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_PADDING
#include <tbb/tbb.h>
#include <tbb/global_control.h>
DISABLE_WARNING_POP

namespace imstk
{
namespace ParallelUtils
{
class ThreadManager
{
public:
    ///
    /// \brief Set system-wide thread pool size for parallel computation
    ///
    static void setThreadPoolSize(const size_t nThreads);

    ///
    /// \brief Set system-wide thread pool size to the optimal value (use all logical cores)
    ///
    static void setOptimalParallelism();

    ///
    /// \brief Returns the size of the thread pool
    ///
    static size_t getThreadPoolSize();

private:
    ///
    /// \brief Global variable for controlling maximum number of worker threads
    ///
    static std::unique_ptr<tbb::global_control> s_tbbGlobalControl;
};
}  // end namespace ParallelUtils
}  // end namespace imstk
