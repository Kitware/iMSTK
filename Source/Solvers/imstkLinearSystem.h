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

#include "imstkNonlinearSystem.h"

namespace imstk
{
///
/// \class LinearSystem
/// \brief Represents the linear system of the form \f$ Ax = b \f$
///
template<typename SystemMatrixType>
class LinearSystem : public NonLinearSystem
{
public:
    ///
    /// \brief Constructor/destructor(s). This class can't be constructed without
    ///     a matrix and rhs. Also, avoid copying this system.
    ///
    LinearSystem() = delete;
    LinearSystem(const LinearSystem&) = delete;
    LinearSystem& operator=(const LinearSystem&) = delete;
    LinearSystem(const SystemMatrixType& matrix, const Vectord& b) : m_A(matrix), m_b(b)
    {
        /*if (m_A.size() == m_b.size())
        {
        m_A = &matrix;
        m_b = &b;
        }
        else
        {
        LOG(ERROR) << "LinearSystem::LinearSystem: The size of the matrix and the r.h.s doesn't match.";
        }*/

        /*this->F = [this](const Vectord& x) -> Vectord&
        {
            this->m_f = this->m_A * x;
            return this->m_f;
        };*/
    }

    ///
    /// \brief Destructor
    ///
    virtual ~LinearSystem(){};

    ///
    ///  \brief Returns a reference to local right hand side vector.
    ///
    const Vectord& getRHSVector() const
    {
        return m_b;
    }

    ///
    /// \brief Set the system rhs corresponding to this system.
    ///
    void setRHSVector(const Vectord& newRhs)
    {
        m_b = newRhs;
    }

    ///
    /// \brief Returns reference to local matrix.
    ///
    const SystemMatrixType& getMatrix() const
    {
        return m_A;
    }

    ///
    /// \brief Set the system matrix corresponding to this ODE system.
    ///
    void setMatrix(const SparseMatrixd& newMatrix)
    {
        m_A = newMatrix;
    }

    ///
    /// \brief Compute the residual as \f$\left \| b-Ax \right \|_2\f$.
    ///
    void computeResidual(const Vectord& x, Vectord& r) const
    {
        r = m_b - m_A * x;
    }

    ///
    /// \brief Returns template expression for the lower triangular part of A.
    ///
    Eigen::TriangularView < SystemMatrixType, Eigen::Lower >
    getLowerTriangular() const;

    ///
    /// \brief Returns template expression for the strict lower triangular part of A.
    ///
    Eigen::TriangularView < SystemMatrixType, Eigen::StrictlyLower >
    getStrictLowerTriangular() const;

    ///
    /// \brief Returns template expression for the upper triangular part of A.
    ///
    Eigen::TriangularView < SystemMatrixType, Eigen::Upper >
    getUpperTrianglular() const;

    ///
    /// \brief Returns template expression for the strict upper triangular part of A.
    ///
    Eigen::TriangularView < SystemMatrixType, Eigen::StrictlyUpper >
    getStrictUpperTriangular() const;

    ///
    /// \brief Get the value of the function F
    ///
    Vectord& getFunctionValue();

    ///
    /// \brief Returns the size of the system
    ///
    size_t getSize();

private:
    const SystemMatrixType& m_A;
    const Vectord& m_b;

    Vectord m_f; ///> Scratch storage for matrix-vector operations
};
} // imstk

