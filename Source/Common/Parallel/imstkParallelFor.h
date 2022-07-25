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
DISABLE_WARNING_POP

namespace imstk
{
namespace ParallelUtils
{
///
/// \brief Execute a function in parallel over a range [beginIdx, endIdx) of indices
/// \param start index
/// \param end index
/// \param function to execute that takes index
/// \param if less than maxN falls back to normal for loop
///
template<class IndexType, class Function>
void
parallelFor(const IndexType beginIdx, const IndexType endIdx, Function&& function, const bool doParallel = true)
{
    if (doParallel)
    {
        tbb::parallel_for(tbb::blocked_range<IndexType>(beginIdx, endIdx),
            [&](const tbb::blocked_range<IndexType>& r) {
                for (IndexType i = r.begin(), iEnd = r.end(); i < iEnd; ++i)
                {
                    function(i);
                }
            });
    }
    else
    {
        for (IndexType i = beginIdx; i < endIdx; i++)
        {
            function(i);
        }
    }
}

///
/// \brief Execute a function in parallel over a range [0, endIdx) of indices
///
template<class IndexType, class Function>
void
parallelFor(const IndexType endIdx, Function&& function, const bool doParallel = true)
{
    parallelFor(IndexType(0), endIdx, std::forward<Function>(function), doParallel);
}

///
/// \brief Execute a 2D function in parallel over a range of indices in the x dimension,
/// indices in the y dimension are scanned sequentially
///
template<class IndexType, class Function>
void
parallelFor2Dx(const IndexType beginX, const IndexType endX,
               const IndexType beginY, const IndexType endY,
               Function&& function)
{
    parallelFor(beginX, endX,
        [&](IndexType i) {
            for (IndexType j = beginY; j < endY; ++j)
            {
                function(i, j);
            }
        });
}

///
/// \brief Execute a 2D function in parallel over a range of indices in the y dimension,
/// indices in the x dimension are scanned sequentially
///
template<class IndexType, class Function>
void
parallelFor2Dy(const IndexType beginX, const IndexType endX,
               const IndexType beginY, const IndexType endY,
               Function&& function)
{
    parallelFor(beginY, endY,
        [&](IndexType j) {
            for (IndexType i = beginX; i < endX; ++i)
            {
                function(i, j);
            }
        });
}

///
/// \brief Execute a 3D function in parallel over a range of indices in the x dimension,
/// indices in the y and z dimensions are scanned sequentially
///
template<class IndexType, class Function>
void
parallelFor3Dx(const IndexType beginX, const IndexType endX,
               const IndexType beginY, const IndexType endY,
               const IndexType beginZ, const IndexType endZ,
               Function&& function)
{
    parallelFor(beginX, endX,
        [&](IndexType i) {
            for (IndexType j = beginY; j < endY; ++j)
            {
                for (IndexType k = beginZ; k < endZ; ++k)
                {
                    function(i, j, k);
                }
            }
        });
}

///
/// \brief Execute a 3D function in parallel over a range of indices in the y dimension,
/// indices in the x and z dimensions are scanned sequentially
///
template<class IndexType, class Function>
void
parallelFor3Dy(const IndexType beginX, const IndexType endX,
               const IndexType beginY, const IndexType endY,
               const IndexType beginZ, const IndexType endZ,
               Function&& function)
{
    parallelFor(beginY, endY,
        [&](IndexType j) {
            for (IndexType i = beginX; i < endX; ++i)
            {
                for (IndexType k = beginZ; k < endZ; ++k)
                {
                    function(i, j, k);
                }
            }
    });
}

///
/// \brief Execute a 3D function in parallel over a range of indices in the z dimension,
/// indices in the x and y dimensions are scanned sequentially
///
template<class IndexType, class Function>
void
parallelFor3Dz(const IndexType beginX, const IndexType endX,
               const IndexType beginY, const IndexType endY,
               const IndexType beginZ, const IndexType endZ,
               Function&& function)
{
    parallelFor(beginX, endX,
        [&](IndexType i) {
            for (IndexType j = beginY; j < endY; ++j)
            {
                for (IndexType k = beginZ; k < endZ; ++k)
                {
                    function(i, j, k);
                }
            }
    });
}
} // end namespace ParallelUtils
} // end namespace imstk
