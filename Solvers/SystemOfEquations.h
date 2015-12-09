// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------
#ifndef SM_SYSTEM_OF_EQUATIONS
#define SM_SYSTEM_OF_EQUATIONS

#include <memory>

//vega includes
#include "sparseMatrix.h"

///
/// \class systemOfEquations
///
/// \brief Base class for system of equations
///
class systemOfEquations
{
public:
    ///
    /// \brief default constructor
    ///
    systemOfEquations();

    ///
    /// \brief destructor
    ///
    ~systemOfEquations();

    ///
    /// \brief Return the size of the matrix
    ///
    virtual int getSize() = 0;
};

///
/// \class LinearSystem
/// \brief linear system \f$ Ax = b \f$
/// \todo Add more matrix properties
///
class LinearSystem : systemOfEquations
{
public:
    ///
    /// \brief default constructor
    ///
    LinearSystem();

    ///
    /// \brief destructor
    ///
    ~LinearSystem();

    ///
    /// \brief Check if the matrix is square
    ///
    virtual bool isSquare() const = 0;

	///
	/// \brief Check if the matrix is symmetric
	///
    virtual bool isSymmetric() const = 0;

	///
	/// \brief Check if the matrix is positive definite
	///
    virtual bool isPositiveDefinite() const = 0;

	///
	/// \brief Check if the matrix is full rank
	///
    virtual bool isFullRank() const = 0;

    // -------------------------------------------------
    //  setters
    // -------------------------------------------------

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getUnknownVector() = 0;

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getForceVector() = 0;

	///
	/// \brief Return the rank
	///
    virtual int getRank() = 0;

	///
	/// \brief Compute the 2-norm of the residue as \f$\left \| b-Ax \right \|_2\f$
	///
    virtual double getResidue2Norm() = 0;
};

///
/// \brief Sparse linear system \f$ Ax = b \f$ using Eigen sparse storage classes
/// \note The linear system should be square
///
class SparseLinearSystem : LinearSystem
{
public:

    ///
    /// \brief default constructor
    ///
    SparseLinearSystem();

    ///
    /// \brief constructor
    ///
    SparseLinearSystem(
        Eigen::SparseMatrix<double>& A,
        Eigen::VectorXd& x,
        Eigen::VectorXd& b) :
        LinearSystem()
    {
        assert(A->rows() == A->cols());

        this->A = A;
        this->x = x;
        this->b = b;
    }

    ///
    /// \brief destructor
    ///
    ~SparseLinearSystem();

    // -------------------------------------------------
    //  getters
    // -------------------------------------------------

    ///
    /// \brief
    ///
    Eigen::SparseMatrix<double>& getMatrix()
    {
        return A;
    }

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getUnknownVector() override
    {
        return x;
    }

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getForceVector() override
    {
        return b;
    }

    ///
    /// \brief Return the size of the matrix
    ///
    virtual int getSize() override
    {
        return A.rows();
    };

private:
    Eigen::SparseMatrix<double> A; ///> sparse matrix
    Eigen::VectorXd x; ///> the l.h.s vector
    Eigen::VectorXd b; ///> the r.h.s vector
};

///
/// \brief Dense (fully stored) linear system Ax=b using Eigen storage classes
/// \todo complete the class implementation
///
class DenseLinearSystem : LinearSystem
{
public:
    ///
    /// \brief default constructor
    ///
    DenseLinearSystem();

    ///
    /// \brief destructor
    ///
    ~DenseLinearSystem();

    // -------------------------------------------------
    //  getters
    // -------------------------------------------------

    ///
    /// \brief
    ///
    virtual Eigen::MatrixXd& getMatrix()
    {
        return A;
    }

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getUnknownVector() override
    {
        return x;
    }

    ///
    /// \brief
    ///
    virtual Eigen::VectorXd& getForceVector() override
    {
        return b;
    }

private:
    Eigen::MatrixXd A; ///> sparse matrix
    Eigen::VectorXd x; ///> the l.h.s vector
    Eigen::VectorXd b; ///> the r.h.s vector
};

///
/// \brief linear system Ax=b
/// \todo complete the class implementation
///
class LcpSystem : systemOfEquations
{
public:
    ///
    /// \brief default constructor
    ///
    LcpSystem();

    ///
    /// \brief destructor
    ///
    ~LcpSystem();

private:
};
#endif // SM_SYSTEM_OF_EQUATIONS