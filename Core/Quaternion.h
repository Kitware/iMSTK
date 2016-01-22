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

#ifndef CORE_QUATERNION_H
#define CORE_QUATERNION_H

// Eigen includes
#include "Eigen/Geometry"

// iMSTK includes
#include "Core/Matrix.h"

/// A quaternion.
template<typename T>
using Quaternion = Eigen::Quaternion<T>;

namespace core
{
/// A quaternion of floats.
using Quaternionf = Quaternion<float>;

/// A quaternion of doubles.
using Quaterniond = Quaternion<double>;
} //core

/// Create a quaternion rotation corresponding to the specified angle (in radians) and axis.
/// \tparam T the numeric data type.
/// \tparam VOpt the option flags
/// \param angle the angle of the rotation, in radians.
/// \param axis the axis of rotation.
/// \returns the rotation quaternion.
template <typename T, int VOpt>
inline Eigen::Quaternion<T> getRotationQuaternion(const T& angle, const Eigen::Matrix<T, 3, 1, VOpt>& axis)
{
    return Eigen::Quaternion<T>(Eigen::AngleAxis<T>(angle, axis));
}

#endif
