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

// STL includes
#include <limits>

#include "smCore/smGeometry.h"
#include "smCore/smRenderDelegate.h"

smAABB::smAABB()
{
    this->reset();
    this->renderDelegate =
      smFactory<smRenderDelegate>::createConcreteClass(
        "smAABBRenderDelegate");
    if (this->renderDelegate)
      this->renderDelegate->setSourceGeometry(this);
}

smVec3d smAABB::center() const
{
    smVec3d output;
    output << 0.5f * ( this->aabbMin[0] + this->aabbMax[0] ),
           0.5f * ( this->aabbMin[1] + this->aabbMax[1] ),
           0.5f * ( this->aabbMin[2] + this->aabbMax[2] );
    return output;
}

bool smAABB::checkOverlap( const smAABB &p_aabbA, const smAABB &p_aabbB )
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

const smAABB &smAABB::operator=( const smAABB &p_aabb )
{
    this->aabbMax = p_aabb.aabbMax;
    this->aabbMin = p_aabb.aabbMin;
    return *this;
}

smAABB &smAABB::operator*( const double p_scale )
{
    this->aabbMin *= p_scale;
    this->aabbMax *= p_scale;
    return *this;
}

void smAABB::subDivide( const double p_length, const int p_divison, smAABB *p_aabb ) const
{
    smInt index = 0;

    for ( smInt ix = 0; ix < p_divison; ix++ )
        for ( smInt iy = 0; iy < p_divison; iy++ )
            for ( smInt iz = 0; iz < p_divison; iz++ )
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

void smAABB::subDivide( const int p_divisionX, const int p_divisionY, const int p_divisionZ, smAABB *p_aabb ) const
{
    double stepX;
    double stepY;
    double stepZ;

    stepX = ( this->aabbMax[0] - this->aabbMin[0] ) / p_divisionX;
    stepY = ( this->aabbMax[1] - this->aabbMin[1] ) / p_divisionY;
    stepZ = ( this->aabbMax[2] - this->aabbMin[2] ) / p_divisionZ;
    smInt index = 0;

    for ( smInt ix = 0; ix < p_divisionX; ix++ )
        for ( smInt iy = 0; iy < p_divisionY; iy++ )
            for ( smInt iz = 0; iz < p_divisionZ; iz++ )
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

void smAABB::subDivide( const int p_division, smAABB *p_aabb ) const
{
    subDivide( p_division, p_division, p_division, p_aabb );
}

double smAABB::halfSizeX() const
{
    return .5 * ( aabbMax[0] - aabbMin[0] );
}

double smAABB::halfSizeY() const
{
    return .5 * ( aabbMax[1] - aabbMin[1] );
}

double smAABB::halfSizeZ() const
{
    return .5 * ( aabbMax[2] - aabbMin[2] );
}

void smAABB::expand( const double &p_factor )
{
    this->aabbMin -= .5 * ( this->aabbMax - this->aabbMin ) * p_factor;
    this->aabbMax += .5 * ( this->aabbMax - this->aabbMin ) * p_factor;
}

smCube::smCube()
{
    center << 0, 0, 0;
    sideLength = 1.0;
}

void smCube::subDivide( int p_divisionPerAxis, smCube *p_cube )
{
    smVec3d minPoint;
    double divLength = ( sideLength / p_divisionPerAxis );
    smInt index = 0;
    minPoint << center[0] - sideLength * 0.5,
             center[1] - sideLength * 0.5,
             center[2] - sideLength * 0.5;

    for ( smInt ix = 0; ix < p_divisionPerAxis; ix++ )
        for ( smInt iy = 0; iy < p_divisionPerAxis; iy++ )
            for ( smInt iz = 0; iz < p_divisionPerAxis; iz++ )
            {
                p_cube[index].center[0] = minPoint[0] + divLength * ix + divLength * 0.5;
                p_cube[index].center[1] = minPoint[1] + divLength * iy + divLength * 0.5;
                p_cube[index].center[2] = minPoint[2] + divLength * iz + divLength * 0.5;
                p_cube[index].sideLength = divLength;


                index++;
            }
}

void smCube::expand( double p_expansion )
{
    sideLength = sideLength + sideLength * p_expansion;
}

smVec3d smCube::leftMinCorner() const
{
    return smVec3d( center[0] - sideLength * 0.5,
                    center[1] - sideLength * 0.5,
                    center[2] - sideLength * 0.5 );
}

smVec3d smCube::rightMaxCorner() const
{
    return smVec3d( center[0] + sideLength * 0.5,
                    center[1] + sideLength * 0.5,
                    center[2] + sideLength * 0.5 );
}

smSphere smCube::getCircumscribedSphere()
{
    return smSphere( center, 0.866025 * sideLength );
}

smSphere smCube::getInscribedSphere()
{
    return smSphere( center, sideLength * 0.5 );
}

smSphere smCube::getTangent2EdgeSphere()
{
    return smSphere( center, sideLength * 0.707106 );
}
