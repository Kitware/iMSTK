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

#include <tbb/tbb.h>

namespace imstk
{
///
/// \brief Execute a function in parallel over a range [beginIdx, endIdx) of indices
///
template<class IndexType, class Function>
void imstk_parallel_for(IndexType beginIdx, IndexType endIdx, Function&& function)
{
    tbb::parallel_for(tbb::blocked_range<IndexType>(beginIdx, endIdx),
        [&](const tbb::blocked_range<IndexType>& r) {
            for(IndexType i = r.begin(), iEnd = r.end(); i < iEnd; ++i)
            {
                function(i);
            }
        });
}

///
/// \brief Execute a function in parallel over a range [0, endIdx) of indices
///
template<class IndexType, class Function>
void imstk_parallel_for(IndexType endIdx, Function&& function)
{
    imstk_parallel_for(IndexType(0), endIdx, std::forward<Function>(function));
}

///
/// \brief Execute a 2D function in parallel over a range of indices in the x dimension,
/// while indices in the y dimension are scanned sequentially
///
template<class IndexType, class Function>
void imstk_parallel_for_2Dx(IndexType beginX, IndexType endX,
                            IndexType beginY, IndexType endY,
                            Function&& function)
{
    imstk_parallel_for(beginX, endX,
        [&](IndexType i) {
            for(IndexType j = beginY; j < endY; ++j)
            {
                function(i, j);
            }
        });
}

///
/// \brief Execute a 2D function in parallel over a range of indices in the y dimension,
/// while indices in the x dimension are scanned sequentially
///
template<class IndexType, class Function>
void imstk_parallel_for_2Dy(IndexType beginX, IndexType endX,
                            IndexType beginY, IndexType endY,
                            Function&& function)
{
    imstk_parallel_for(beginY, endY,
        [&](IndexType j) {
            for(IndexType i = beginX; i < endX; ++i)
            {
                function(i, j);
            }
        });
}

///
/// \brief Execute a 3D function in parallel over a range of indices in the x dimension,
/// while indices in the y and z dimensions are scanned sequentially
///
template<class IndexType, class Function>
void imstk_parallel_for_3Dx(IndexType beginX, IndexType endX,
                            IndexType beginY, IndexType endY,
                            IndexType beginZ, IndexType endZ,
                            Function&& function)
{
    imstk_parallel_for(beginX, endX,
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
/// \brief Execute a 3D function in parallel over a range of indices in the y dimension,
/// while indices in the x and z dimensions are scanned sequentially
///
template<class IndexType, class Function>
void imstk_parallel_for_3Dy(IndexType beginX, IndexType endX,
                            IndexType beginY, IndexType endY,
                            IndexType beginZ, IndexType endZ,
                            Function&& function)
{
    imstk_parallel_for(beginY, endY,
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
/// \brief Execute a 3D function in parallel over a range of indices in the z dimension,
/// while indices in the x and y dimensions are scanned sequentially
///
template<class IndexType, class Function>
void imstk_parallel_for_3Dz(IndexType beginX, IndexType endX,
                            IndexType beginY, IndexType endY,
                            IndexType beginZ, IndexType endZ,
                            Function&& function)
{
    imstk_parallel_for(beginX, endX,
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
} // end namespace imstk
