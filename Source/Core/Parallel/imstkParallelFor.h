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

// Define this macro to not use tbb in for loop
//#define NO_TBB

#ifndef NO_TBB
#  include <tbb/tbb.h>
#endif

namespace imstk
{
namespace ParallelFor
{
///
/// \brief Execute a for loop in parallel
///
template<class IndexType, class Function>
void run(IndexType beginIdx, IndexType endIdx, Function&& function)
{
#ifdef NO_TBB
    for(IndexType i = beginIdx; i < endIdx; ++i)
    {
        function(i);
    }
#else
    tbb::parallel_for(tbb::blocked_range<IndexType>(beginIdx, endIdx),
        [&](const tbb::blocked_range<IndexType>& r) {
                for(IndexType i = r.begin(), iEnd = r.end(); i < iEnd; ++i)
                {
                    function(i);
                }
        });
#endif
}

///
/// \brief Execute a for loop in parallel
///
template<class IndexType, class Function>
void run(IndexType endIdx, Function&& function)
{
    ParallelFor::run(IndexType(0), endIdx, std::forward<Function>(function));
}

///
/// \brief Execute a for loop, the first dimension is run in parallel
///
template<class IndexType, class Function>
void run_parallel_x(IndexType beginX, IndexType endX,
                    IndexType beginY, IndexType endY,
                    Function&& function)
{
    ParallelFor::run(beginX, endX,
        [&](IndexType i) {
                for(IndexType j = beginY; j < endY; ++j)
                {
                    function(i, j);
                }
        });
}

///
/// \brief Execute a for loop, the second dimension is run in parallel
///
template<class IndexType, class Function>
void run_parallel_y(IndexType beginX, IndexType endX,
                    IndexType beginY, IndexType endY,
                    Function&& function)
{
    ParallelFor::run(beginY, endY,
        [&](IndexType j) {
                for(IndexType i = beginX; i < endX; ++i)
                {
                    function(i, j);
                }
        });
}

///
/// \brief Execute a for loop, the first dimension is run in parallel
///
template<class IndexType, class Function>
void run_parallel_x(IndexType beginX, IndexType endX,
                    IndexType beginY, IndexType endY,
                    IndexType beginZ, IndexType endZ,
                    Function&& function)
{
    ParallelFor::run(beginX, endX,
        [&](IndexType i) {
                for(IndexType j = beginY; j < endY; ++j)
                {
                    for(IndexType k = beginZ; k < endZ; ++k)
                    {
                        function(i, j, k);
                    }
                }
        });
}

///
/// \brief Execute a for loop, the second dimension is run in parallel
///
template<class IndexType, class Function>
void run_parallel_y(IndexType beginX, IndexType endX,
                    IndexType beginY, IndexType endY,
                    IndexType beginZ, IndexType endZ,
                    Function&& function)
{
    ParallelFor::run(beginY, endY,
        [&](IndexType j) {
                for(IndexType i = beginX; i < endX; ++i)
                {
                    for(IndexType k = beginZ; k < endZ; ++k)
                    {
                        function(i, j, k);
                    }
                }
    });
}

///
/// \brief Execute a for loop, the third dimension is run in parallel
///
template<class IndexType, class Function>
void run_parallel_z(IndexType beginX, IndexType endX,
                    IndexType beginY, IndexType endY,
                    IndexType beginZ, IndexType endZ,
                    Function&& function)
{
    ParallelFor::run(beginX, endX,
        [&](IndexType i) {
                for(IndexType j = beginY; j < endY; ++j)
                {
                    for(IndexType k = beginZ; k < endZ; ++k)
                    {
                        function(i, j, k);
                    }
                }
    });
}
} // end namespace ParallelFor
} // end namespace imstk
