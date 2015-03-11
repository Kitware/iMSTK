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

#ifndef SM_COLLISIONMOLLER_H
#define SM_COLLISIONMOLLER_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smGeometry.h"
#include "smUtilities/smVector.h"

/// \brief Collision utililites
class smCollisionMoller
{
public:
    /// \brief triangle and triangle collision retursn intersection and projection points
    static smBool tri2tri(smVec3f &p_tri1Point1,
                          smVec3f &p_tri1Point2,
                          smVec3f &p_tri1Point3,
                          smVec3f &p_tri2Point1,
                          smVec3f &p_tri2Point2,
                          smVec3f &p_tri2Point3,
                          smInt &coplanar,
                          smVec3f &p_intersectionPoint1,
                          smVec3f &p_intersectionPoint2,
                          smShort &p_tri1SinglePointIndex,
                          smShort &p_tri2SinglePointIndex,
                          smVec3f &p_projPoint1,
                          smVec3f &p_projPoint2);

    /// \brief checks if the two triangles intersect
    static smBool tri2tri(smVec3f &p_tri1Point1,
                          smVec3f &p_tri1Point2,
                          smVec3f &p_tri1Point3,
                          smVec3f &p_tri2Point1,
                          smVec3f &p_tri2Point2,
                          smVec3f &p_tri2Point3);

    /// \brief check if the two AABB overlap returns encapsulating AABB of two
    static bool checkOverlapAABBAABB(smAABB &aabbA, smAABB &aabbB, smAABB &result);

    /// \brief check the AABBs overlap. returns true if they intersect
    static bool checkOverlapAABBAABB(smAABB &aabbA, smAABB &aabbB);

    /// \brief check if the point p_vertex is inside the AABB
    static inline bool checkOverlapAABBAABB(smAABB &aabbA, smVec3f &p_vertex);

    /// \brief  checks if the line intersects the tirangle. returns if it is true. the intersection is returned in  p_interSection
    static smBool checkLineTri(smVec3f &p_linePoint1,
                               smVec3f &p_linePoint2,
                               smVec3f &p_tri1Point1,
                               smVec3f &p_tri1Point2,
                               smVec3f &p_tri1Point3,
                               smVec3f &p_interSection);

    /// \brief checks if the triangles points are within the AABB
    static smBool checkAABBTriangle(smAABB &p_aabb, smVec3f &v1, smVec3f &v2, smVec3f &v3);

    static smBool checkAABBPoint(const smAABB &p_aabb, const smVec3f &p_v);
};

#endif // SM_COLLISIONMOLLER_H
