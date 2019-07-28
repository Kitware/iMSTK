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

#pragma once

/// \todo Remove this in TBB 2019 Update 4: https://github.com/intel/tbb/blob/tbb_2019/CHANGES#L117
#define TBB_PREVIEW_GLOBAL_CONTROL 1

#include <tbb/tbb.h>
#include <tbb/global_control.h>

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

private:
    ///
    /// \brief Global variable for controlling maximum number of worker threads
    ///
    static std::unique_ptr<tbb::global_control> s_tbbGlobalControl;
};
}  // end namespace ParallelUtils
}  // end namespace imstk
