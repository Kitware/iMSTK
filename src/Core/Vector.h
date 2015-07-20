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

// STL includes
#include <vector>

// Eigen includes
#include <Eigen/Core>
#include <Eigen/Geometry>

// namespace SimMedTK
// {


/// Aliases for vectors

/// A 2D vector
template<typename T>
using Vector2 = Eigen::Matrix<T,2,1>;

/// A 3D vector
template<typename T>
using Vector3 = Eigen::Matrix<T,3,1>;

/// A 4D vector
template<typename T>
using Vector4 = Eigen::Matrix<T,4,1>;

/// A 6D vector
template<typename T>
using Vector6 = Eigen::Matrix<T,6,1>;

/// A dynamic size column vector
template<typename T>
using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

// STD vectors of vectors
template<typename T>
using StdVector3 = std::vector<Vector3<T>,
    Eigen::aligned_allocator<Vector3<T>>>;

namespace core
{
/// A 2D vector of floats.
using Vec2f = Vector2<float>;

/// A 3D vector of floats.
using Vec3f = Vector3<float>;

/// A 4D vector of floats.
using Vec4f = Vector4<float>;

/// A 6D vector of floats.
using Vec6f = Vector6<float>;

/// A 2D vector of doubles.
using Vec2d = Vector2<double>;

/// A 3D vector of doubles.
using Vec3d = Vector3<double>;

/// A 4D vector of doubles.
using Vec4d = Vector4<double>;

/// A 6D matrix of doubles.
using Vec6d = Vector6<double>;

/// A dynamic size column vector
using Vectorf = Vector<float>;

/// A dynamic size column vector
using Vectord = Vector<double>;

// STL vectors of float vectors
using StdVector3f = StdVector3<float>;

// STL vectors of double vectors
using StdVector3d = StdVector3<double>;

template<typename T>
void flattenVectorArray(const StdVector3<T> &v, Eigen::Matrix<T, 3, Eigen::Dynamic> &out)
{
    out.resize(v.size());
    for(size_t i = 0, end = v.size(); i < end; ++i)
    {
        out.col(i) = v(i);
    }
}

} // core
// } // SimMedTK

#endif // SMVECTOR_H
