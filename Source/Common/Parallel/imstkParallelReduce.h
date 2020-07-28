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
namespace ParallelUtils
{
///
/// \brief Private helper class, providing operator() using in std::parallel_reduce
///  for finding range of a container with specified begin/end/operator[],operator++ functions
///
template<class ContainerType>
class RangeFunctor
{
public:
    RangeFunctor(const ContainerType& data) : m_Data(data) { }
    RangeFunctor(RangeFunctor& pObj, tbb::split) : m_Data(pObj.m_Data) { }

    // Prohibit copying
    RangeFunctor() = delete;
    RangeFunctor& operator=(const RangeFunctor&) = delete;

    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for (size_t i = r.begin(); i != r.end(); i++)
        {
            m_Max = m_Max > m_Data[i] ? m_Max : m_Data[i];
            m_Min = m_Max < m_Data[i] ? m_Min : m_Data[i];
        }
    }

    void join(RangeFunctor& pObj)
    {
        m_Max = m_Max > pObj.m_Max ? m_Max : pObj.m_Max;
        m_Min = m_Min < pObj.m_Min ? m_Min : pObj.m_Min;
    }

    Vec2d getRange() const { return Vec2d(m_Min, m_Max); }

private:
    Real m_Min = std::numeric_limits<Real>::max();
    Real m_Max = std::numeric_limits<Real>::min();
    const ContainerType& m_Data;
};

///
/// \brief Private helper class, providing operator() using in std::parallel_reduce
///  for finding max L2 norm of an array of Vec3r
///
class MaxL2NormFunctor
{
public:
    MaxL2NormFunctor(const StdVectorOfVec3r& data) : m_Data(data) {}
    MaxL2NormFunctor(MaxL2NormFunctor& pObj, tbb::split) : m_Data(pObj.m_Data) {}

    // Prohibit copying
    MaxL2NormFunctor() = delete;
    MaxL2NormFunctor& operator=(const MaxL2NormFunctor&) = delete;

    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for (size_t i = r.begin(); i != r.end(); ++i)
        {
            Real mag2 = m_Data[i].squaredNorm();
            m_Result = m_Result > mag2 ? m_Result : mag2;
        }
    }

    void join(MaxL2NormFunctor& pObj) { m_Result = m_Result > pObj.m_Result ? m_Result : pObj.m_Result; }
    Real getResult() const { return std::sqrt(m_Result); }

private:
    Real m_Result = 0;
    const StdVectorOfVec3r& m_Data;
};

///
/// \brief Private helper class, providing operator() using in std::parallel_reduce
///  for finding axis-aligned bounding box of a point set
///
class AABBFunctor
{
public:
    AABBFunctor(const StdVectorOfVec3r& data) : m_Data(data) { if (data.size() > 0) { m_UpperCorner = data[0]; } }
    AABBFunctor(AABBFunctor& pObj, tbb::split) : m_Data(pObj.m_Data) {}

    // Prohibit copying
    AABBFunctor() = delete;
    AABBFunctor& operator=(const AABBFunctor&) = delete;

    ///
    /// \brief Compute the lower and upper corner of \p this in range \p r
    ///
    void operator()(const tbb::blocked_range<size_t>& r)
    {
        for (size_t i = r.begin(); i != r.end(); ++i)
        {
            const auto& vec = m_Data[i];
            for (int j = 0; j < 3; ++j)
            {
                m_LowerCorner[j] = (m_LowerCorner[j] < vec[j]) ? m_LowerCorner[j] : vec[j];
                m_UpperCorner[j] = (m_UpperCorner[j] > vec[j]) ? m_UpperCorner[j] : vec[j];
            }
        }
    }

    ///
    /// \brief Compute the AABB of \p this and anohter object \p pObj as a whole
    ///
    void join(AABBFunctor& pObj)
    {
        for (int j = 0; j < 3; ++j)
        {
            m_LowerCorner[j] = (m_LowerCorner[j] < pObj.m_LowerCorner[j]) ? m_LowerCorner[j] : pObj.m_LowerCorner[j];
            m_UpperCorner[j] = (m_UpperCorner[j] > pObj.m_UpperCorner[j]) ? m_UpperCorner[j] : pObj.m_UpperCorner[j];
        }
    }

    ///
    /// \brief Get the lower corner
    ///
    const Vec3r& getLowerCorner() const { return m_LowerCorner; }
    ///
    /// \brief Get the upper corner
    ///
    const Vec3r& getUpperCorner() const { return m_UpperCorner; }

private:
    Vec3r m_LowerCorner = Vec3r(std::numeric_limits<Real>::max(),
                                std::numeric_limits<Real>::max(),
                                std::numeric_limits<Real>::max());
    Vec3r m_UpperCorner = Vec3r(-std::numeric_limits<Real>::max(),
                                -std::numeric_limits<Real>::max(),
                                -std::numeric_limits<Real>::max());
    const StdVectorOfVec3r& m_Data;
};

///
/// \brief Find the maximum value of L2 norm from the input data array
///
inline Real
findMaxL2Norm(const StdVectorOfVec3r& data)
{
    MaxL2NormFunctor pObj(data);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, data.size()), pObj);
    return pObj.getResult();
}

///
/// \brief Find the bounding box of a point set
///
inline void
findAABB(const StdVectorOfVec3r& points, Vec3r& lowerCorner, Vec3r& upperCorner)
{
    AABBFunctor pObj(points);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, points.size()), pObj);
    lowerCorner = pObj.getLowerCorner();
    upperCorner = pObj.getUpperCorner();
}
} // end namespace ParallelUtils
} // end namespace imstk
