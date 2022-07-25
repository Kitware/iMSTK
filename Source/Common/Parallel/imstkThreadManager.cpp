/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkThreadManager.h"
#include "imstkLogger.h"

namespace imstk
{
namespace ParallelUtils
{
std::unique_ptr<tbb::global_control> ThreadManager::s_tbbGlobalControl;

void
ThreadManager::setThreadPoolSize(const size_t nThreads)
{
    LOG_IF(FATAL, (nThreads == 0)) << "Invalid number of threads";
    LOG(INFO) << "Set number of worker threads to " << nThreads;

    if (s_tbbGlobalControl)
    {
        s_tbbGlobalControl.reset();
    }

    s_tbbGlobalControl = std::unique_ptr<tbb::global_control>(
                new tbb::global_control(tbb::global_control::max_allowed_parallelism,
                                        nThreads));
}

void
ThreadManager::setOptimalParallelism()
{
    setThreadPoolSize(static_cast<size_t>(tbb::info::default_concurrency()));
}

size_t
ThreadManager::getThreadPoolSize()
{
    return s_tbbGlobalControl->active_value(tbb::global_control::max_allowed_parallelism);
}
}  // end namespace ParallelUtils
}  // end namespace imstk
