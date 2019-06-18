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

#include "imstkMath.h"
#include <tbb/tbb.h>

#undef min
#undef max

namespace imstk
{
///
/// \brief The ParallelReduce class
/// \brief A class for executing reduce operations in parallel
///
class ParallelReduce
{
using Vec3r = Eigen::Matrix<Real, 3, 1>;
using StdVT_Vec3r = std::vector<Vec3r, Eigen::aligned_allocator<Vec3r>>;

///
/// \brief Private helper class for finding max norm
///
class ParallelMaxNorm
{
public:
    ParallelMaxNorm(const StdVT_Vec3r& data) : m_Data(data) {}
    ParallelMaxNorm(ParallelMaxNorm& pObj, tbb::split) : m_Data(pObj.m_Data) {}

    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            Real mag2 = m_Data[i].squaredNorm();
            m_Result = m_Result > mag2 ? m_Result : mag2;
        }
    }

    void join(ParallelMaxNorm& pObj) { m_Result = m_Result > pObj.m_Result ? m_Result : pObj.m_Result; }
    Real getResult() const noexcept { return std::sqrt(m_Result); }

private:
    Real m_Result = 0;
    const StdVT_Vec3r& m_Data;
};

///
/// \brief Private helper class for finding AABB of a point set
///
class ParallelAABB
{
public:
    ParallelAABB(const StdVT_Vec3r& data) : m_Data(data) { if(data.size() > 0) { m_UpperCorner = data[0]; } }
    ParallelAABB(ParallelAABB& pObj, tbb::split) : m_Data(pObj.m_Data) {}

    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            const auto& vec = m_Data[i];
            for(int j = 0; j < 3; ++j)
            {
                m_LowerCorner[j] = (m_LowerCorner[j] < vec[j]) ? m_LowerCorner[j] : vec[j];
                m_UpperCorner[j] = (m_UpperCorner[j] > vec[j]) ? m_UpperCorner[j] : vec[j];
            }
        }
    }

    void join(ParallelAABB& pObj)
    {
        for(int j = 0; j < 3; ++j)
        {
            m_LowerCorner[j] = (m_LowerCorner[j] < pObj.m_LowerCorner[j]) ? m_LowerCorner[j] : pObj.m_LowerCorner[j];
            m_UpperCorner[j] = (m_UpperCorner[j] > pObj.m_UpperCorner[j]) ? m_UpperCorner[j] : pObj.m_UpperCorner[j];
        }
    }

    const Vec3r& getLowerCorner() const { return m_LowerCorner; }
    const Vec3r& getUpperCorner() const { return m_UpperCorner; }

private:
    Vec3r m_LowerCorner = Vec3r(std::numeric_limits<Real>::max(),
                                std::numeric_limits<Real>::max(),
                                std::numeric_limits<Real>::max());
    Vec3r m_UpperCorner = Vec3r(-std::numeric_limits<Real>::max(),
                                -std::numeric_limits<Real>::max(),
                                -std::numeric_limits<Real>::max());
    const StdVT_Vec3r& m_Data;
};

public:
    ///
    /// \brief Find the maximum value of vector magnitude (|| v ||) for each vector v in the input data array
    ///
    static Real getMaxNorm(const StdVT_Vec3r& data)
    {
        ParallelMaxNorm pObj(data);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, data.size()), pObj);
        return pObj.getResult();
    }

    ///
    /// \brief Find the bounding box of a point set
    ///
    static void getAABB(const StdVT_Vec3r& points, Vec3r& lowerCorner, Vec3r& upperCorner)
    {
        ParallelAABB pObj(points);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, points.size()), pObj);
        lowerCorner = pObj.getLowerCorner();
        upperCorner = pObj.getUpperCorner();
    }
};
} // end namespace imstk
