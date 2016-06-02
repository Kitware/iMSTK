// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef imstkLinearSystem_h
#define imstkLinearSystem_h

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
    LinearSystem(const SystemMatrixType& matrix, const Vectord& b);

    ///
    /// \brief Destructor
    ///
    virtual ~LinearSystem() = default;

    ///
    ///  \brief Returns a reference to local right hand side vector.
    ///
    const Vectord& getRHSVector() const;

    ///
    /// \brief Set the system rhs corresponding to this system.
    ///
    void setRHSVector(const Vectord& newRhs);

    ///
    /// \brief Returns reference to local matrix.
    ///
    const SystemMatrixType& getMatrix() const;

    ///
    /// \brief Set the system matrix corresponding to this ODE system.
    ///
    void setMatrix(const SparseMatrixd& newMatrix);

    ///
    /// \brief Compute the residual as \f$\left \| b-Ax \right \|_2\f$.
    ///
    Vectord& computeResidual(const Vectord& x, Vectord& r) const;

    ///
    /// \brief Returns template expression for the lower triangular part of A.
    ///
    Eigen::SparseTriangularView < SystemMatrixType, Eigen::Lower >
        getLowerTriangular() const;

    ///
    /// \brief Returns template expression for the strict lower triangular part of A.
    ///
    Eigen::SparseTriangularView < SystemMatrixType, Eigen::StrictlyLower >
        getStrictLowerTriangular() const;

    ///
    /// \brief Returns template expression for the upper triangular part of A.
    ///
    Eigen::SparseTriangularView < SystemMatrixType, Eigen::Upper >
        getUpperTrianglular() const;

    ///
    /// \brief Returns template expression for the strict upper triangular part of A.
    ///
    Eigen::SparseTriangularView < SystemMatrixType, Eigen::StrictlyUpper >
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

}

#endif // imstkLinearSystem_h
