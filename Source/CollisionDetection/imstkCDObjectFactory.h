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

#include "imstkCollisionDetection.h"

namespace imstk
{
class Geometry;

///
/// \brief Static factory for collision detection sub classes
/// If the collision pair is PointSet to SurfaceMesh, or SurfaceMesh to SurfaceMesh,
/// it will be added to an internal static octree for detecting collision
/// \todo Other collision pair may be considered to use octree too
///
extern std::shared_ptr<CollisionDetection> makeCollisionDetectionObject(const CollisionDetection::Type type,
                                                                        std::shared_ptr<Geometry>      collidingGeometryA,
                                                                        std::shared_ptr<Geometry>      collidingGeometryB,
                                                                        std::shared_ptr<CollisionData> colData);
}
