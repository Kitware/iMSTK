// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//            Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef SMMATRIX_H
#define SMMATRIX_H

// STL includes
#include<vector>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Geometry>

// namespace SimMedTK
// {
// namespace Math
// {

/// A 2x2 matrix
template<typename T>
using Matrix22 = Eigen::Matrix<T, 2, 2>;

/// A 3x3 matrix
template<typename T>
using Matrix33 = Eigen::Matrix<T, 3, 3>;

/// A 4x4 matrix
template<typename T>
using Matrix44 = Eigen::Matrix<T, 4, 4>;

/// A 4x4 matrix
template<typename T>
using Matrix66 = Eigen::Matrix<T, 6, 6>;

/// A dynamic size diagonal matrix
template<typename T>
using DiagonalMatrix = Eigen::DiagonalMatrix<T, Eigen::Dynamic>;

/// A dynamic size dense matrix
template<typename T>
using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

/// A dynamic size sparse column-major matrix
// template<typename T, int StorageType = Eigen::ColMajor>
// using SparseMatrix = Eigen::SparseMatrix<T,StorageType>;

/// A 2x2 matrix of floats.
using Matrix22f = Matrix22<float>;

/// A 3x3 matrix of floats.
using Matrix33f = Matrix33<float>;

/// A 4x4 matrix of floats.
using Matrix44f = Matrix44<float>;

/// A 2x2 matrix of doubles.
using Matrix22d = Matrix22<double>;

/// A 3x3 matrix of doubles.
using Matrix33d = Matrix33<double>;

/// A 4x4 matrix of doubles.
using Matrix44d = Matrix44<double>;

/// A 6x6 matrix of doubles.
using Matrix66d = Matrix66<double>;

/// A dynamic size diagonal matrix of floats
using DiagonalMatrixf = DiagonalMatrix<float>;

/// A dynamic size diagonal matrix of doubles
using DiagonalMatrixd = DiagonalMatrix<double>;

/// A dynamic size matrix of floats
using Matrixf = Matrix<float>;

/// A dynamic size matrix of doubles
using Matrixd = Matrix<double>;

template<typename T, int StorageType>
void fillSparseMatrix(const std::vector<Eigen::Triplet<T>> &triplets, Eigen::SparseMatrix<T,StorageType> &A)
{
    A.setFromTriplets(triplets.begin(),triplets.end());
}

template<typename T, int StorageType, int opt>
void solveSparseSystemCholesky(const Eigen::SparseMatrix<T,StorageType> &A,
                               const Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &b,
                               Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &x)
{
    // Solving:
    Eigen::SimplicialCholesky<Eigen::SparseMatrix<T,StorageType>> solver(A);  // performs a Cholesky factorization of A
    x = solver.solve(b);         // use the factorization to solve for the given right hand side
}

// WARNING: The input matrix A should be in a compressed and column-major form. Otherwise an expensive copy will be made.
// template<typename T, int StorageType, int opt>
// void solveSparseSystemLU(const Eigen::SparseMatrix<T,StorageType> &A,
//                          const Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &b,
//                          Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &x)
// {
//     // Solving:
//     Eigen::SparseLU<Eigen::SparseMatrix<T,StorageType>,
//         Eigen::COLAMDOrdering<Eigen::SparseMatrix<T,StorageType>::Index> > solver;
//     // Compute the ordering permutation vector from the structural pattern of A
//     solver.analyzePattern(A);
//     // Compute the numerical factorization
//     solver.factorize(A);
//     //Use the factors to solve the linear system
//     x = solver.solve(b);
// }

template<typename T, int StorageType, int opt>
void conjugateGradient(const Eigen::SparseMatrix<T,StorageType> &A,
                       const Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &b,
                       Eigen::Matrix<T, Eigen::Dynamic, 1, opt> &x)
{
    // Solving:
    Eigen::ConjugateGradient<Eigen::SparseMatrix<T,StorageType>> solver;
    solver.compute(A);
    x = solver.solve(b);
}


// } // Math
// } // SimMedTK


#endif // SMMATRIX_H
