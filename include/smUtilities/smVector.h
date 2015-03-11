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

#ifndef SMVECTOR_H
#define SMVECTOR_H

#include <Eigen/Core>


// namespace SimMedTK
// {
// namespace Math
// {

/// A 2D vector of floats.
typedef Eigen::Matrix<float,  2, 1>  smVec2f;

/// A 3D vector of floats.
typedef Eigen::Matrix<float,  3, 1>  smVec3f;

/// A 4D vector of floats.
typedef Eigen::Matrix<float,  4, 1>  smVec4f;

/// A 6D vector of floats.
typedef Eigen::Matrix<float,  6, 1>  smVec6f;

/// A 2D vector of doubles.
typedef Eigen::Matrix<double, 2, 1>  smVec2d;

/// A 3D vector of doubles.
typedef Eigen::Matrix<double, 3, 1>  smVec3d;

/// A 4D vector of doubles.
typedef Eigen::Matrix<double, 4, 1>  smVec4d;

/// A 6D matrix of doubles.
typedef Eigen::Matrix<double, 6, 1> smVec6d;

/// A dynamic size column vector
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> smVector;


// } // Math
// } // SimMedTK

#endif // SMVECTOR_H
