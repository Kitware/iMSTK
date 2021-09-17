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

#define NOMINMAX

#include <Eigen/Geometry>

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif
#include <Eigen/Sparse>
#ifdef WIN32
#pragma warning( pop )
#endif

#include <Eigen/StdVector>
#include <vector>
#include <memory>

#ifndef _MSC_VER
namespace std
{
template<typename T, typename ... Args>
std::unique_ptr<T>
make_unique(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args) ...));
}
}
#endif

namespace imstk
{
// 2D vector
// using Vec2f = Eigen::Vector2f;
// using Vec2d = Eigen::Vector2d;
using Vec2f = Eigen::Matrix<float, 2, 1>;
using Vec2d = Eigen::Matrix<double, 2, 1>;
using Vec2i = Eigen::Matrix<int, 2, 1>;
using StdVectorOfVec2f = std::vector<Vec2f, Eigen::aligned_allocator<Vec2f>>;
using StdVectorOfVec2d = std::vector<Vec2d, Eigen::aligned_allocator<Vec2d>>;

// 3D vector
// using Vec3f = Eigen::Vector3f;
// using Vec3d = Eigen::Vector3d;
using Vec3f = Eigen::Matrix<float, 3, 1>;
using Vec3d = Eigen::Matrix<double, 3, 1>;
using Vec3i = Eigen::Matrix<int, 3, 1>;
using StdVectorOfVec3f = std::vector<Vec3f, Eigen::aligned_allocator<Vec3f>>;
using StdVectorOfVec3d = std::vector<Vec3d, Eigen::aligned_allocator<Vec3d>>;

// 4D vector
// using Vec4f = Eigen::Vector4f;
// using Vec4d = Eigen::Vector4d;
using Vec4f = Eigen::Matrix<float, 4, 1>;
using Vec4d = Eigen::Matrix<double, 4, 1>;
using Vec4i = Eigen::Matrix<int, 4, 1>;
using StdVectorOfVec4f = std::vector<Vec4f, Eigen::aligned_allocator<Vec4f>>;
using StdVectorOfVec4d = std::vector<Vec4d, Eigen::aligned_allocator<Vec4d>>;

// 6D vector
using Vec6f = Eigen::Matrix<float, 6, 1>;
using Vec6d = Eigen::Matrix<double, 6, 1>;
using Vec6i = Eigen::Matrix<int, 6, 1>;

// 8D vector
using Vec8i = Eigen::Matrix<int, 8, 1>;

// Dynamic size vector
using Vectorf = Eigen::VectorXf;
using Vectord = Eigen::VectorXd;
using StdVectorOfVectorf = std::vector<Vectorf, Eigen::aligned_allocator<Vectorf>>;
using StdVectorOfVectord = std::vector<Vectord, Eigen::aligned_allocator<Vectord>>;

// Quaternion
using Quatf = Eigen::Quaternion<float>;
using Quatd = Eigen::Quaternion<double>;
using StdVectorOfQuatf = std::vector<Quatf, Eigen::aligned_allocator<Quatf>>;
using StdVectorOfQuatd = std::vector<Quatd, Eigen::aligned_allocator<Quatd>>;

// Angle-Axis
using Rotf = Eigen::AngleAxisf;
using Rotd = Eigen::AngleAxisd;

// 3x3 Matrix
using Mat3f = Eigen::Matrix<float, 3, 3>;
using Mat3d = Eigen::Matrix<double, 3, 3>;
using StdVectorOfMat3d = std::vector<Mat3d, Eigen::aligned_allocator<Mat3d>>;

// 4x4 Matrix
using Mat4f = Eigen::Matrix<float, 4, 4> ;
using Mat4d = Eigen::Matrix<double, 4, 4>;

/// A dynamic size matrix of floats
using Matrixf = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

/// A dynamic size matrix of doubles
using Matrixd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

// A dynamic size sparse matrix of doubles
using SparseMatrixf = Eigen::SparseMatrix<float, Eigen::RowMajor>;

// A dynamic size sparse matrix of doubles
using SparseMatrixd = Eigen::SparseMatrix<double, Eigen::RowMajor>;

// Rigid transform (translation and rotation)
using RigidTransform3f = Eigen::Isometry3f;
using RigidTransform3d = Eigen::Isometry3d;

// Affine transform (translation, rotation, scaling and shearing)
using AffineTransform3f = Eigen::Affine3f;
using AffineTransform3d = Eigen::Affine3d;

// Handy Cartesian vectors in 3d
#define UP_VECTOR       Vec3d(0.0, 1.0, 0.0)
#define DOWN_VECTOR     Vec3d(0, -1, 0)
#define RIGHT_VECTOR    Vec3d(1, 0, 0)
#define LEFT_VECTOR     Vec3d(-1, 0, 0)
#define FORWARD_VECTOR  Vec3d(0, 0, -1)
#define BACKWARD_VECTOR Vec3d(0, 0, 1)
#define WORLD_ORIGIN    Vec3d::Zero()

/// Some commonly used math constants
#define PI               3.14159265358979323846
#define PI_2             1.57079632679489661923
#define PI_4             0.785398163397448309616
#define INV_1_PI         0.318309886183790671538
#define INV_2_PI         0.636619772367581343076
#define TWO_OVER_SQRT_PI 1.12837916709551257390
#define SQRT2            1.41421356237309504880
#define SQRT1_2          0.707106781186547524401
#define NLOG_E           2.71828182845904523536
#define LOG2E            1.44269504088896340736
#define LOG10E           0.434294481903251827651
#define LN2              0.693147180559945309417
#define LN10             2.30258509299404568402

#define MAX_D                std::numeric_limits<double>::max()
#define MIN_D                std::numeric_limits<double>::min()
#define VERY_SMALL_EPSILON_D std::numeric_limits<double>::epsilon()

#define MAX_F                std::numeric_limits<float>::max()
#define MIN_F                std::numeric_limits<float>::min()
#define VERY_SMALL_EPSILON_F std::numeric_limits<float>::epsilon()

static inline Mat4d
mat4dTranslate(const Vec3d& translate)
{
    return AffineTransform3d(Eigen::Translation3d(translate)).matrix();
}

static inline Mat4d
mat4dScale(const Vec3d& scale)
{
    Mat4d mScale = Mat4d::Identity();
    mScale(0, 0) = scale[0];
    mScale(1, 1) = scale[1];
    mScale(2, 2) = scale[2];
    return mScale;
}

static inline Mat4d
mat4dRotation(const Quatd& rotation)
{
    Mat4d mRot = Mat4d::Identity();
    mRot.block<3, 3>(0, 0) = rotation.toRotationMatrix();
    return mRot;
}

static inline Mat4d
mat4dRotation(const Rotd& rotation)
{
    Mat4d mRot = Mat4d::Identity();
    mRot.block<3, 3>(0, 0) = rotation.toRotationMatrix();
    return mRot;
}

static inline Mat4d
mat4dRotation(const Mat3d& rotation)
{
    Mat4d mRot = Mat4d::Identity();
    mRot.block<3, 3>(0, 0) = rotation;
    return mRot;
}

///
/// \brief Translation, Rotation, Scaling decomposition, ignores shears
///
static inline void
mat4dTRS(const Mat4d& m, Vec3d& t, Mat3d& r, Vec3d& s)
{
    // Assumes affine, no shear
    const Vec3d& x = m.block<3, 1>(0, 0);
    const Vec3d& y = m.block<3, 1>(0, 1);
    const Vec3d& z = m.block<3, 1>(0, 2);

    s = Vec3d(m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());

    r = Mat3d::Identity();
    r.block<3, 1>(0, 0) = x.normalized();
    r.block<3, 1>(0, 1) = y.normalized();
    r.block<3, 1>(0, 2) = z.normalized();

    t = m.block<3, 1>(0, 3);
}

///
/// \brief Computes tet signed volume/determinant
///
static double
tetVolume(Vec3d p0, Vec3d p1, Vec3d p2, Vec3d p3)
{
    Mat4d m;
    m.block<1, 3>(0, 0) = p0;
    m.block<1, 3>(1, 0) = p1;
    m.block<1, 3>(2, 0) = p2;
    m.block<1, 3>(3, 0) = p3;
    m(0, 3) = m(1, 3) = m(2, 3) = m(3, 3) = 1.0;
    return m.determinant() / 6.0;
}

///
/// \brief Compute bary centric coordinates (u,v,w) given triangle in 2d space (and point p on triangle)
///
static Vec3d
baryCentric(const Vec2d& p, const Vec2d& a, const Vec2d& b, const Vec2d& c)
{
    const Vec2d  v0  = b - a;
    const Vec2d  v1  = c - a;
    const Vec2d  v2  = p - a;
    const double den = v0[0] * v1[1] - v1[0] * v0[1];
    const double v   = (v2[0] * v1[1] - v1[0] * v2[1]) / den;
    const double w   = (v0[0] * v2[1] - v2[0] * v0[1]) / den;
    const double u   = 1.0 - v - w;
    return Vec3d(u, v, w);
}

///
/// \brief Compute bary centric coordinates (u,v,w) of point p, given 3 points in 3d space (a,b,c)
///
static Vec3d
baryCentric(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c)
{
    const Vec3d  v0    = b - a;
    const Vec3d  v1    = c - a;
    const Vec3d  v2    = p - a;
    const double d00   = v0.dot(v0);
    const double d01   = v0.dot(v1);
    const double d11   = v1.dot(v1);
    const double d20   = v2.dot(v0);
    const double d21   = v2.dot(v1);
    const double denom = d00 * d11 - d01 * d01;
    const double v     = (d11 * d20 - d01 * d21) / denom;
    const double w     = (d00 * d21 - d01 * d20) / denom;
    const double u     = 1.0 - v - w;
    return Vec3d(u, v, w);
}

///
/// \brief Compute bary centric coordinates (u,v,w,x) of point p, given 4 points in 3d space (a,b,c,d)
///
static Vec4d
baryCentric(const Vec3d& p, const Vec3d& a, const Vec3d& b, const Vec3d& c, const Vec3d& d)
{
    Mat4d A;
    A << a[0], a[1], a[2], 1.0,
        b[0], b[1], b[2], 1.0,
        c[0], c[1], c[2], 1.0,
        d[0], d[1], d[2], 1.0;

    Vec4d  weights;
    double det = A.determinant(); // Signed volume
    for (int i = 0; i < 4; ++i)
    {
        Mat4d B = A;
        B(i, 0)    = p[0];
        B(i, 1)    = p[1];
        B(i, 2)    = p[2];
        weights[i] = B.determinant() / det;
    }
    return weights;
}

///
/// \brief Cantors pairing function, take two ints and produce a unique one
/// The resulting numbers are close for two close A's and B's
///
template<typename T>
static T
cantor(const T a, const T b)
{
    return (a + b) * (a + b + 1) / 2 + b;
}

///
/// \brief Similar to cantors pairing function but "commutative", take two ints and produce a unique one
/// For two inputs a and b, f(a,b)==f(b,a)
/// The resulting numbers are close for two close A's and B's
///
template<typename T>
static T
symCantor(const T a, const T b)
{
    const T max = std::max(a, b);
    const T min = std::min(a, b);
    return max * (max + 1) / 2 + min;
}
}