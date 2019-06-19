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

#ifndef imstkMath_h
#define imstkMath_h

#include <vector>

#include <Eigen/Geometry>
#include <Eigen/Sparse>
#include <Eigen/StdVector>

namespace imstk
{
// Define Real type and dependent types
using Real = double;
using Vec2r = Eigen::Matrix<Real, 2, 1>;
using Vec3r = Eigen::Matrix<Real, 3, 1>;
using Vec4r = Eigen::Matrix<Real, 4, 1>;
using StdVectorOfReal  = std::vector<Real>;
using StdVectorOfVec2r = std::vector<Vec2r, Eigen::aligned_allocator<Vec2r>>;
using StdVectorOfVec3r = std::vector<Vec3r, Eigen::aligned_allocator<Vec3r>>;
using StdVectorOfVec4r = std::vector<Vec4r, Eigen::aligned_allocator<Vec4r>>;

// 2D vector
using Vec2f = Eigen::Vector2f;
using Vec2d = Eigen::Vector2d;
using StdVectorOfVec2f = std::vector<Vec2f, Eigen::aligned_allocator<Vec2f>>;
using StdVectorOfVec2d = std::vector<Vec2d, Eigen::aligned_allocator<Vec2d>>;

// 3D vector
using Vec3f = Eigen::Vector3f;
using Vec3d = Eigen::Vector3d;
using StdVectorOfVec3f = std::vector<Vec3f, Eigen::aligned_allocator<Vec3f>>;
using StdVectorOfVec3d = std::vector<Vec3d, Eigen::aligned_allocator<Vec3d>>;

// 4D vector
using Vec4f = Eigen::Vector4f;
using Vec4d = Eigen::Vector4d;
using StdVectorOfVec4f = std::vector<Vec4f, Eigen::aligned_allocator<Vec4f>>;
using StdVectorOfVec4d = std::vector<Vec4d, Eigen::aligned_allocator<Vec4d>>;

// Dynamic size vector
using Vectorf = Eigen::VectorXf;
using Vectord = Eigen::VectorXd;
using StdVectorOfVectorf = std::vector<Vectorf, Eigen::aligned_allocator<Vectorf>>;
using StdVectorOfVectord = std::vector<Vectord, Eigen::aligned_allocator<Vectord>>;

// Quaternion
using Quatf = Eigen::Quaternion<float, Eigen::DontAlign>;
using Quatd = Eigen::Quaternion<double, Eigen::DontAlign>;

// Angle-Axis
using Rotf = Eigen::AngleAxisf;
using Rotd = Eigen::AngleAxisd;

// 3x3 Matrix
using Mat3f = Eigen::Matrix3f;
using Mat3d = Eigen::Matrix3d;

// 4x4 Matrix
using Mat4f = Eigen::Matrix4f;
using Mat4d = Eigen::Matrix4d;

/// A dynamic size matrix of floats
using Matrixf = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

/// A dynamic size matrix of doubles
using Matrixd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

// A dynamic size sparse matrix of doubles
using SparseMatrixf = Eigen::SparseMatrix < float, Eigen::RowMajor >;

// A dynamic size sparse matrix of doubles
using SparseMatrixd = Eigen::SparseMatrix < double, Eigen::RowMajor >;

// Rigid transform (translation and rotation)
using RigidTransform3f = Eigen::Isometry3f;
using RigidTransform3d = Eigen::Isometry3d;

// Affine transform (translation, rotation, scaling and shearing)
using AffineTransform3f = Eigen::Affine3f;
using AffineTransform3d = Eigen::Affine3d;

// Handy Cartesian vectors in 3d
#define UP_VECTOR Vec3d(0.0, 1.0, 0.0)
#define DOWN_VECTOR Vec3d(0, -1, 0)
#define RIGHT_VECTOR Vec3d(1, 0, 0)
#define LEFT_VECTOR Vec3d(-1, 0, 0)
#define FORWARD_VECTOR Vec3d(0, 0, -1)
#define BACKWARD_VECTOR Vec3d(0, 0, 1)
#define WORLD_ORIGIN Vec3d::Zero()

/// Some commonly used math constants
static constexpr Real PI = 3.14159265358979323846;
static constexpr Real PI_2 = 1.57079632679489661923;
static constexpr Real PI_4 = 0.785398163397448309616;
static constexpr Real INV_1_PI = 0.318309886183790671538;
static constexpr Real INV_2_PI = 0.636619772367581343076;
static constexpr Real TWO_OVER_SQRTPI = 1.12837916709551257390;
static constexpr Real SQRT2 = 1.41421356237309504880;
static constexpr Real SQRT1_2 = 0.707106781186547524401;
static constexpr Real NLOG_E = 2.71828182845904523536;
static constexpr Real LOG2E = 1.44269504088896340736;
static constexpr Real LOG10E = 0.434294481903251827651;
static constexpr Real LN2 = 0.693147180559945309417;
static constexpr Real LN10 = 2.30258509299404568402;

static constexpr double MAX_REAL = std::numeric_limits<Real>::max();
static constexpr double MIN_REAL = std::numeric_limits<Real>::min();
static constexpr double MAX_D = std::numeric_limits<double>::max();
static constexpr double MIN_D = std::numeric_limits<double>::min();
static constexpr float MAX_F = std::numeric_limits<float>::max();
static constexpr float MIN_F = std::numeric_limits<float>::min();

static constexpr Real MACHINE_PRECISION = std::numeric_limits<Real>::denorm_min();
static constexpr double MACHINE_PRECISION_D = std::numeric_limits<double>::denorm_min();
static constexpr double MACHINE_PRECISION_F = std::numeric_limits<double>::denorm_min();
}
#endif // ifndef imstkMath_h
