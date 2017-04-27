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

#include "imstkLinearSystem.h"

#include <g3log/g3log.hpp>

namespace imstk
{
//template<typename SystemMatrixType>
//LinearSystem<SystemMatrixType>::LinearSystem(const SystemMatrixType &matrix, const Vectord &b)
//{
//    if (m_A.size() == m_b.size())
//    {
//        m_A = &matrix;
//        m_b = &b;
//    }
//    else
//    {
//        LOG(ERROR) << "LinearSystem::LinearSystem: The size of the matrix and the r.h.s doesn't match.";
//    }
//
//    this->F = [this](const Vectord& x) -> Vectord&
//    {
//        this->m_f = this->m_A * x;
//        return this->m_f;
//    };
//}

template<typename SystemMatrixType>
Eigen::TriangularView < SystemMatrixType, Eigen::Lower >
LinearSystem<SystemMatrixType>::getLowerTriangular() const
{
    return m_A.template triangularView<Eigen::Lower>();
}

template<typename SystemMatrixType>
Eigen::TriangularView < SystemMatrixType, Eigen::StrictlyLower >
LinearSystem<SystemMatrixType>::getStrictLowerTriangular() const
{
    return m_A.template triangularView<Eigen::StrictlyLower>();
}

template<typename SystemMatrixType>
Eigen::TriangularView < SystemMatrixType, Eigen::Upper >
LinearSystem<SystemMatrixType>::getUpperTrianglular() const
{
    return m_A.template triangularView<Eigen::Upper>();
}

template<typename SystemMatrixType>
Eigen::TriangularView < SystemMatrixType, Eigen::StrictlyUpper >
LinearSystem<SystemMatrixType>::getStrictUpperTriangular() const
{
    return m_A.template triangularView<Eigen::StrictlyUpper>();
}

template<typename SystemMatrixType>
Vectord&
LinearSystem<SystemMatrixType>::getFunctionValue()
{
    return m_f;
}

template<typename SystemMatrixType> size_t
LinearSystem<SystemMatrixType>::getSize()
{
    return m_A.size();
}
} //imstk