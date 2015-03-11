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
    inline smFloat distance(smVec3f p_vector);
    inline smVec3f project(smVec3f p_vector);
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
    inline smAABB()
    {
        this->aabbMin << 0, 0, 0;
        this->aabbMax << 0, 0, 0;
    }
    /// \brief center of the AABB
    inline smVec3f center() const
    {
        smVec3f output;
        output << 0.5f*(this->aabbMin[0] + this->aabbMax[0]),
                  0.5f*(this->aabbMin[1] + this->aabbMax[1]),
                  0.5f*(this->aabbMin[2] + this->aabbMax[2]);
        return output;
    }

    /// \brief check if two AABB overlaps
    static inline smBool checkOverlap(const smAABB &p_aabbA, const smAABB &p_aabbB)
    {

        if (p_aabbA.aabbMin[0] > p_aabbB.aabbMax[0] ||
            p_aabbA.aabbMax[0] < p_aabbB.aabbMin[0] ||
            p_aabbA.aabbMin[1] > p_aabbB.aabbMax[1] ||
            p_aabbA.aabbMax[1] < p_aabbB.aabbMin[1] ||
            p_aabbA.aabbMin[2] > p_aabbB.aabbMax[2] ||
            p_aabbA.aabbMax[2] < p_aabbB.aabbMin[2])
        {
            return false;
        }
        return true;
    }

    /// \brief set  p_aabb to the current one
    inline smAABB &operator=(const smAABB &p_aabb)
    {
        this->aabbMax = p_aabb.aabbMax;
        this->aabbMin = p_aabb.aabbMin;
        return *this;
    }

    /// \brief scale the AABB
    inline smAABB &operator*(const smFloat p_scale)
    {
        this->aabbMin *= p_scale;
        this->aabbMax *= p_scale;
        return *this;
    }

    /// \brief sub divides p_length will be used to create the slices
    void subDivide(const smFloat p_length, const smInt p_divison, smAABB *p_aabb) const
    {
        smInt index = 0;

        for (smInt ix = 0; ix < p_divison; ix++)
            for (smInt iy = 0; iy < p_divison; iy++)
                for (smInt iz = 0; iz < p_divison; iz++)
                {
                    p_aabb[index].aabbMin[0] = this->aabbMin[0] + p_length * ix;
                    p_aabb[index].aabbMin[1] = this->aabbMin[1] + p_length * iy;
                    p_aabb[index].aabbMin[2] = this->aabbMin[2] + p_length * iz;

                    p_aabb[index].aabbMax[0] = p_aabb[index].aabbMin[0] + p_length;
                    p_aabb[index].aabbMax[1] = p_aabb[index].aabbMin[1] + p_length;
                    p_aabb[index].aabbMax[2] = p_aabb[index].aabbMin[2] + p_length;
                    index++;
                }
    }

    /// \brief divides current AABB in x,y,z axes with specificed divisions. results are placed in p_aabb
    void subDivide(const smInt p_divisionX, const smInt p_divisionY, const smInt p_divisionZ, smAABB *p_aabb) const
    {
        smFloat stepX;
        smFloat stepY;
        smFloat stepZ;

        stepX = (this->aabbMax[0] - this->aabbMin[0]) / p_divisionX;
        stepY = (this->aabbMax[1] - this->aabbMin[1]) / p_divisionY;
        stepZ = (this->aabbMax[2] - this->aabbMin[2]) / p_divisionZ;
        smInt index = 0;

        for (smInt ix = 0; ix < p_divisionX; ix++)
            for (smInt iy = 0; iy < p_divisionY; iy++)
                for (smInt iz = 0; iz < p_divisionZ; iz++)
                {

                    p_aabb[index].aabbMin[0] = this->aabbMin[0] + stepX * ix;
                    p_aabb[index].aabbMin[1] = this->aabbMin[1] + stepY * iy;
                    p_aabb[index].aabbMin[2] = this->aabbMin[2] + stepZ * iz;

                    p_aabb[index].aabbMax[0] = p_aabb[index].aabbMin[0] + stepX;
                    p_aabb[index].aabbMax[1] = p_aabb[index].aabbMin[1] + stepY;
                    p_aabb[index].aabbMax[2] = p_aabb[index].aabbMin[2] + stepZ;
                    index++;
                }
    }

    /// \brief divides current AABB in all axes with specificed p_division. results are placed in p_aabb
    inline void subDivide(const smInt p_division, smAABB *p_aabb) const
    {
        subDivide(p_division, p_division, p_division, p_aabb);
    }

    /// \brief returns half of X edge of AABB
    inline smFloat halfSizeX() const
    {
        return .5*(aabbMax[0] - aabbMin[0]);
    }

    /// \brief returns half of Y edge of AABB
    inline smFloat halfSizeY() const
    {
        return .5*(aabbMax[1] - aabbMin[1]);
    }

    /// \brief returns half of Z edge of AABB
    inline smFloat halfSizeZ() const
    {
        return .5*(aabbMax[2] - aabbMin[2]);
    }

    /// \brief expands aabb with p_factor
    inline void expand(const smFloat &p_factor)
    {
        this->aabbMin -= .5*(this->aabbMax - this->aabbMin)*p_factor;
        this->aabbMax += .5*(this->aabbMax - this->aabbMin)*p_factor;
    }
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
    inline smSphere()
    {
        center << 0, 0, 0;
        radius = 1.0;
    }
    /// \brief sphere constructor with center and radius
    inline smSphere(smVec3f p_center, smFloat p_radius)
    {
        radius = p_radius;
        center = p_center;
    }
};

/// \brief cube
struct smCube
{
    /// \brief cube center
    smVec3f center;
    /// \brief cube length
    smFloat sideLength;

    /// \brief constructor
    inline smCube()
    {
        center << 0, 0, 0;
        sideLength = 1.0;
    }
    /// \brief subdivides the cube in mulitple cube with given number of cubes identified for each axis with p_divisionPerAxis
    inline void subDivide(smInt p_divisionPerAxis, smCube*p_cube)
    {
        smVec3f minPoint;
        smFloat divLength = (sideLength / p_divisionPerAxis);
        smInt index = 0;
        minPoint << center[0] - sideLength * 0.5,
                    center[1] - sideLength * 0.5,
                    center[2] - sideLength * 0.5;

        for (smInt ix = 0; ix < p_divisionPerAxis; ix++)
            for (smInt iy = 0; iy < p_divisionPerAxis; iy++)
                for (smInt iz = 0; iz < p_divisionPerAxis; iz++)
                {
                    p_cube[index].center[0] = minPoint[0] + divLength * ix + divLength * 0.5;
                    p_cube[index].center[1] = minPoint[1] + divLength * iy + divLength * 0.5;
                    p_cube[index].center[2] = minPoint[2] + divLength * iz + divLength * 0.5;
                    p_cube[index].sideLength = divLength;


                    index++;
                }
    }
    /// \brief expands the cube. increases the edge length with expansion*edge length
    inline void expand(smFloat p_expansion)
    {
        sideLength = sideLength + sideLength * p_expansion;
    }
    /// \brief returns the left most corner
    inline smVec3f leftMinCorner()
    {
        return smVec3f(center[0] - sideLength * 0.5,
                       center[1] - sideLength * 0.5,
                       center[2] - sideLength * 0.5);
    }
    /// \brief returns right most corner
    inline smVec3f rightMaxCorner()
    {
        return smVec3f(center[0] + sideLength * 0.5,
                       center[1] + sideLength * 0.5,
                       center[2] + sideLength * 0.5);
    }
    /// \brief returns the smallest sphere encapsulates the cube
    inline smSphere getCircumscribedSphere()
    {
        return smSphere(center, 0.866025 * sideLength);
    }
    /// \brief returns the  sphere with half edge of the cube as a radius
    inline smSphere getInscribedSphere()
    {
        return smSphere(center, sideLength * 0.5);
    }
    /// \brief get tangent sphere
    inline smSphere getTangent2EdgeSphere()
    {
        return smSphere(center, sideLength * 0.707106);
    }
};

#endif
