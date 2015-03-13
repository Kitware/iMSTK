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

#ifndef SMGEOMETRY_H
#define SMGEOMETRY_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smUtilities/smVector.h"

//forward declaration
struct smSphere;

/// \brief  Simple Plane definition with unit normal and spatial location
struct smPlane
{
    smVec3f unitNormal;
    smVec3f pos;
    smFloat distance(smVec3f p_vector);
    smVec3f project(smVec3f p_vector);
};

/// \brief Axis Aligned bounding box declarions
class smAABB
{
public:
    /// \brief minimum x,y,z point
    smVec3f aabbMin;

    /// \brief maximum x,y,z point
    smVec3f aabbMax;

    /// \brief constrcutor. The default is set to origin for aabbMin and aabbMax
    smAABB();

    /// \brief center of the AABB
    smVec3f center() const;

    /// \brief check if two AABB overlaps
    static smBool checkOverlap(const smAABB &p_aabbA, const smAABB &p_aabbB);

    /// \brief set  p_aabb to the current one
    smAABB &operator=(const smAABB &p_aabb);

    /// \brief scale the AABB
    smAABB &operator*(const smFloat p_scale);

    /// \brief sub divides p_length will be used to create the slices
    void subDivide(const smFloat p_length, const smInt p_divison, smAABB *p_aabb) const;

    /// \brief divides current AABB in x,y,z axes with specificed divisions. results are placed in p_aabb
    void subDivide(const smInt p_divisionX, const smInt p_divisionY, const smInt p_divisionZ, smAABB *p_aabb) const;

    /// \brief divides current AABB in all axes with specificed p_division. results are placed in p_aabb
    void subDivide(const smInt p_division, smAABB *p_aabb) const;

    /// \brief returns half of X edge of AABB
    smFloat halfSizeX() const;

    /// \brief returns half of Y edge of AABB
    smFloat halfSizeY() const;

    /// \brief returns half of Z edge of AABB
    smFloat halfSizeZ() const;

    /// \brief expands aabb with p_factor
    void expand(const smFloat &p_factor);
};

/// \brief sphere structure
struct smSphere
{

public:
    /// \brief center of sphere
    smVec3f center;

    /// \brief radius of sshere
    smFloat radius;

    /// \brief constructor
    smSphere();

    /// \brief sphere constructor with center and radius
    smSphere(smVec3f p_center, smFloat p_radius);
};

/// \brief cube
struct smCube
{
    /// \brief cube center
    smVec3f center;

    /// \brief cube length
    smFloat sideLength;

    /// \brief constructor
    smCube();

    /// \brief subdivides the cube in mulitple cube with given number of cubes identified for each axis with p_divisionPerAxis
    void subDivide(smInt p_divisionPerAxis, smCube*p_cube);

    /// \brief expands the cube. increases the edge length with expansion*edge length
    void expand(smFloat p_expansion);

    /// \brief returns the left most corner
    smVec3f leftMinCorner();

    /// \brief returns right most corner
    smVec3f rightMaxCorner();

    /// \brief returns the smallest sphere encapsulates the cube
    smSphere getCircumscribedSphere();

    /// \brief returns the  sphere with half edge of the cube as a radius
    smSphere getInscribedSphere();

    /// \brief get tangent sphere
    smSphere getTangent2EdgeSphere();
};

#endif
