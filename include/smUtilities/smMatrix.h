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

#ifndef SMMATRIX_H
#define SMMATRIX_H

#include <Eigen/Core>

// namespace SimMedTK
// {
// namespace Math
// {

/// A 2x2 matrix of floats.
typedef Eigen::Matrix<float,  2, 2> smMatrix22f;

/// A 3x3 matrix of floats.
typedef Eigen::Matrix<float,  3, 3> smMatrix33f;

/// A 4x4 matrix of floats.
typedef Eigen::Matrix<float,  4, 4> smMatrix44f;

/// A 2x2 matrix of doubles.
typedef Eigen::Matrix<double, 2, 2> smMatrix22d;

/// A 3x3 matrix of doubles.
typedef Eigen::Matrix<double, 3, 3> smMatrix33d;

/// A 4x4 matrix of doubles.
typedef Eigen::Matrix<double, 4, 4> smMatrix44d;

/// A 6x6 matrix of doubles.
typedef Eigen::Matrix<double, 6, 6> smMatrix66d;

/// A dynamic size diagonal matrix
typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic> smDiagonalMatrix;

/// A dynamic size matrix
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> smMatrix;


// } // Math
// } // SimMedTK


#endif // SMMATRIX_H
