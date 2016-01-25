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

#ifndef COLLISION_COLLISIONMOLLER_H
#define COLLISION_COLLISIONMOLLER_H

// iMSTK includes
#include "Core/Config.h"
#include "Core/Geometry.h"
#include "Core/Vector.h"

namespace imstk {

/// \brief Collision utilities
class CollisionMoller
{
public:
    /// \brief triangle and triangle collision retursn intersection and projection points
    static bool tri2tri(Vec3d &p_tri1Point1,
                        Vec3d &p_tri1Point2,
                        Vec3d &p_tri1Point3,
                        Vec3d &p_tri2Point1,
                        Vec3d &p_tri2Point2,
                        Vec3d &p_tri2Point3,
                        int &coplanar,
                        Vec3d &p_intersectionPoint1,
                        Vec3d &p_intersectionPoint2,
                        short &p_tri1SinglePointIndex,
                        short &p_tri2SinglePointIndex,
                        Vec3d &p_projPoint1,
                        Vec3d &p_projPoint2);

    static bool tri2tri(Vec3d &p_tri1Point1,
                        Vec3d &p_tri1Point2,
                        Vec3d &p_tri1Point3,
                        Vec3d &p_tri2Point1,
                        Vec3d &p_tri2Point2,
                        Vec3d &p_tri2Point3,
                        double &depth,
                        Vec3d &contactPoint,
                        Vec3d &normal);

    /// \brief checks if the two triangles intersect
    static bool tri2tri(Vec3d &p_tri1Point1,
                        Vec3d &p_tri1Point2,
                        Vec3d &p_tri1Point3,
                        Vec3d &p_tri2Point1,
                        Vec3d &p_tri2Point2,
                        Vec3d &p_tri2Point3);

    /// \brief check if the two AABB overlap returns encapsulating AABB of two
    static bool checkOverlapAABBAABB(AABB &aabbA, AABB &aabbB, AABB &result);

    /// \brief check the AABBs overlap. returns true if they intersect
    static bool checkOverlapAABBAABB(const AABB &aabbA, const AABB &aabbB);

    /// \brief check if the point p_vertex is inside the AABB
    static inline bool checkOverlapAABBAABB(AABB &aabbA, Vec3d &p_vertex);

    /// \brief  checks if the line intersects the tirangle. returns if it is true. the intersection is returned in  p_interSection
    static bool checkLineTri(Vec3d &p_linePoint1,
                             Vec3d &p_linePoint2,
                             Vec3d &p_tri1Point1,
                             Vec3d &p_tri1Point2,
                             Vec3d &p_tri1Point3,
                             Vec3d &p_interSection);

    /// \brief checks if the triangles points are within the AABB
    static bool checkAABBTriangle(AABB &p_aabb, Vec3d &v1, Vec3d &v2, Vec3d &v3);

    static bool checkAABBPoint(const AABB &p_aabb, const Vec3d &p_v);
};

}

#endif // COLLISION_COLLISIONMOLLER_H
