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

// STL includes
#include <limits>

#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"

AABB::AABB()
{
    this->reset();
    this->renderDelegate =
      Factory<RenderDelegate>::createConcreteClass(
        "AABBRenderDelegate");
    if (this->renderDelegate)
      this->renderDelegate->setSourceGeometry(this);
}

core::Vec3d AABB::center() const
{
    core::Vec3d output;
    output << 0.5f * ( this->aabbMin[0] + this->aabbMax[0] ),
           0.5f * ( this->aabbMin[1] + this->aabbMax[1] ),
           0.5f * ( this->aabbMin[2] + this->aabbMax[2] );
    return output;
}

bool AABB::checkOverlap( const AABB &p_aabbA, const AABB &p_aabbB )
{

    if ( p_aabbA.aabbMin[0] > p_aabbB.aabbMax[0] ||
            p_aabbA.aabbMax[0] < p_aabbB.aabbMin[0] ||
            p_aabbA.aabbMin[1] > p_aabbB.aabbMax[1] ||
            p_aabbA.aabbMax[1] < p_aabbB.aabbMin[1] ||
            p_aabbA.aabbMin[2] > p_aabbB.aabbMax[2] ||
            p_aabbA.aabbMax[2] < p_aabbB.aabbMin[2] )
    {
        return false;
    }
    return true;
}

bool AABB::overlaps( const AABB &other ) const
{

    if ( this->aabbMin[0] > other.aabbMax[0] ||
        this->aabbMax[0] < other.aabbMin[0] ||
        this->aabbMin[1] > other.aabbMax[1] ||
        this->aabbMax[1] < other.aabbMin[1] ||
        this->aabbMin[2] > other.aabbMax[2] ||
        this->aabbMax[2] < other.aabbMin[2] )
    {
        return false;
    }
    return true;
}

const AABB &AABB::operator=( const AABB &p_aabb )
{
    this->aabbMax = p_aabb.aabbMax;
    this->aabbMin = p_aabb.aabbMin;
    return *this;
}

AABB &AABB::operator*( const double p_scale )
{
    this->aabbMin *= p_scale;
    this->aabbMax *= p_scale;
    return *this;
}

void AABB::subDivide( const double p_length, const int p_divison, AABB *p_aabb ) const
{
    int index = 0;

    for ( int ix = 0; ix < p_divison; ix++ )
        for ( int iy = 0; iy < p_divison; iy++ )
            for ( int iz = 0; iz < p_divison; iz++ )
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

void AABB::subDivide( const int p_divisionX, const int p_divisionY, const int p_divisionZ, AABB *p_aabb ) const
{
    double stepX;
    double stepY;
    double stepZ;

    stepX = ( this->aabbMax[0] - this->aabbMin[0] ) / p_divisionX;
    stepY = ( this->aabbMax[1] - this->aabbMin[1] ) / p_divisionY;
    stepZ = ( this->aabbMax[2] - this->aabbMin[2] ) / p_divisionZ;
    int index = 0;

    for ( int ix = 0; ix < p_divisionX; ix++ )
        for ( int iy = 0; iy < p_divisionY; iy++ )
            for ( int iz = 0; iz < p_divisionZ; iz++ )
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

void AABB::subDivide( const int p_division, AABB *p_aabb ) const
{
    subDivide( p_division, p_division, p_division, p_aabb );
}

double AABB::halfSizeX() const
{
    return .5 * ( aabbMax[0] - aabbMin[0] );
}

double AABB::halfSizeY() const
{
    return .5 * ( aabbMax[1] - aabbMin[1] );
}

double AABB::halfSizeZ() const
{
    return .5 * ( aabbMax[2] - aabbMin[2] );
}

void AABB::expand( const double &p_factor )
{
    this->aabbMin -= .5 * ( this->aabbMax - this->aabbMin ) * p_factor;
    this->aabbMax += .5 * ( this->aabbMax - this->aabbMin ) * p_factor;
}

Cube::Cube()
{
    center << 0, 0, 0;
    sideLength = 1.0;
}

void Cube::subDivide( int p_divisionPerAxis, Cube *p_cube )
{
    core::Vec3d minPoint;
    double divLength = ( sideLength / p_divisionPerAxis );
    int index = 0;
    minPoint << center[0] - sideLength * 0.5,
             center[1] - sideLength * 0.5,
             center[2] - sideLength * 0.5;

    for ( int ix = 0; ix < p_divisionPerAxis; ix++ )
        for ( int iy = 0; iy < p_divisionPerAxis; iy++ )
            for ( int iz = 0; iz < p_divisionPerAxis; iz++ )
            {
                p_cube[index].center[0] = minPoint[0] + divLength * ix + divLength * 0.5;
                p_cube[index].center[1] = minPoint[1] + divLength * iy + divLength * 0.5;
                p_cube[index].center[2] = minPoint[2] + divLength * iz + divLength * 0.5;
                p_cube[index].sideLength = divLength;


                index++;
            }
}

void Cube::expand( double p_expansion )
{
    sideLength = sideLength + sideLength * p_expansion;
}

core::Vec3d Cube::leftMinCorner() const
{
    return core::Vec3d( center[0] - sideLength * 0.5,
                    center[1] - sideLength * 0.5,
                    center[2] - sideLength * 0.5 );
}

core::Vec3d Cube::rightMaxCorner() const
{
    return core::Vec3d( center[0] + sideLength * 0.5,
                    center[1] + sideLength * 0.5,
                    center[2] + sideLength * 0.5 );
}

Sphere Cube::getCircumscribedSphere()
{
    return Sphere( center, 0.866025 * sideLength );
}

Sphere Cube::getInscribedSphere()
{
    return Sphere( center, sideLength * 0.5 );
}

Sphere Cube::getTangent2EdgeSphere()
{
    return Sphere( center, sideLength * 0.707106 );
}
