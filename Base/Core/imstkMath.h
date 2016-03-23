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

// Dynamic size vector
using VecNf = Eigen::VectorXf;
using VecNd = Eigen::VectorXd;

// Quaternion
using Quatf = Eigen::Quaternionf;
using Quatd = Eigen::Quaterniond;

// 3x3 Matrix
using Mat3f = Eigen::Matrix<float, 3, 3>;
using Mat3d = Eigen::Matrix<double, 3, 3>;

#define UP Vec3d(0.0, 1.0, 0.0)
#define DOWN Vec3d(0, -1, 0)
#define RIGHT Vec3d(1, 0, 0)
#define LEFT Vec3d(-1, 0, 0)
#define FORWARD Vec3d(0, 0, 1)
#define BACKWARD Vec3d(0, 0, -1)
#define ORIGIN Vec3d(0, 0, 0)
}

#endif // ifndef imstkMath_h
