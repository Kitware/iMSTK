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

namespace imstk {
// 2D vector
using Vec2f = Eigen::Vector2f;
using Vec2d = Eigen::Vector2d;

// 3D vector
using Vec3f = Eigen::Vector3f;
using Vec3d = Eigen::Vector3d;

// 4D vector
using Vec4f = Eigen::Vector4f;
using Vec4d = Eigen::Vector4d;

// Dynamic size vector
using VecNf = Eigen::VectorXf;
using VecNd = Eigen::VectorXd;

// Quaternion
using Quatf = Eigen::Quaternionf;
using Quatd = Eigen::Quaterniond;

// 3x3 Matrix
using Mat3f = Eigen::Matrix3f;
using Mat3d = Eigen::Matrix3d;

// Rigid transform (translation and rotation)
using RigidTransform3f = Eigen::Isometry3f;
using RigidTransform3d = Eigen::Isometry3d;

// Affine transform (translation, rotation, scaling and shearing)
using AffineTransform3f = Eigen::Affine3f;
using AffineTransform3d = Eigen::Affine3d;

// Handy cartesian vectors in 3d
#define UP_VECTOR Vec3d(0.0, 1.0, 0.0)
#define DOWN_VECTOR Vec3d(0, -1, 0)
#define RIGHT_VECTOR Vec3d(1, 0, 0)
#define LEFT_VECTOR Vec3d(-1, 0, 0)
#define FORWARD_VECTOR Vec3d(0, 0, -1)
#define BACKWARD_VECTOR Vec3d(0, 0, 1)
#define WORLD_ORIGIN Vec3d::Zero()

// Some commonly used math constants
const double PI = 3.14159265358979323846;
const double PI_2 = 1.57079632679489661923;
const double PI_4 = 0.785398163397448309616;
const double INV_1_PI = 0.318309886183790671538;
const double INV_2_PI = 0.636619772367581343076;
const double TWO_OVER_SQRTPI = 1.12837916709551257390;
const double SQRT2 = 1.41421356237309504880;
const double SQRT1_2 = 0.707106781186547524401;
const double NLOG_E = 2.71828182845904523536;
const double LOG2E = 1.44269504088896340736;
const double LOG10E = 0.434294481903251827651;
const double LN2 = 0.693147180559945309417;
const double LN10 = 2.30258509299404568402;

}

#endif // ifndef imstkMath_h
