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

#include "imstkThreadManager.h"
#include <g3log/g3log.hpp>

namespace imstk
{
namespace ParallelUtils
{
std::unique_ptr<tbb::global_control> ThreadManager::s_tbbGlobalControl;

void ThreadManager::setThreadPoolSize(const size_t nThreads)
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

void ThreadManager::setOptimalParallelism()
{
    setThreadPoolSize(static_cast<size_t>(tbb::task_scheduler_init::default_num_threads()));
}
}  // end namespace ParallelUtils
}  // end namespace imstk
