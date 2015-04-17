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

// STD includes
#include <cassert>

// SimMedTK includes
#include "smCollision/smOctreeCell.h"
#include "smCollision/smCollisionMoller.h"

smOctreeCell::smOctreeCell() : BaseType()
{

}

smOctreeCell::~smOctreeCell()
{

}

smVec3f &smOctreeCell::getCenter()
{
    return  cube.center;
}

const smVec3f &smOctreeCell::getCenter() const
{
    return  cube.center;
}

void smOctreeCell::setCenter( const smVec3f &center )
{
    cube.center = center;
}

float &smOctreeCell::getLength()
{
    return cube.sideLength;
}

const float &smOctreeCell::getLength() const
{
    return cube.sideLength;
}

void smOctreeCell::setLength( const float length )
{
    cube.sideLength = length;
}

void smOctreeCell::copyShape( const smOctreeCell &cell )
{
    cube = cell.cube;
}

void smOctreeCell::expand( const float expandScale )
{
    cube.expand( expandScale );
}

bool smOctreeCell::isCollidedWithTri( smVec3f &v0, smVec3f &v1, smVec3f &v2 )
{
    smAABB tempAABB;
    tempAABB.aabbMin = cube.leftMinCorner();
    tempAABB.aabbMax = cube.rightMaxCorner();
    return smCollisionMoller::checkAABBTriangle( tempAABB, v0, v1, v2 );
}

bool smOctreeCell::isCollidedWithPoint()
{
    std::cerr << "Error::smOctreeCell::isCollidedWithPoint(): Function not implemented." << std::endl;
    return 0;
}

void smOctreeCell::subDivide( const int divisionPerAxis,
                              std::array<smOctreeCell,smOctreeCell::numberOfSubdivisions> &cells )
{
    size_t totalCubes = divisionPerAxis * divisionPerAxis * divisionPerAxis;

    assert( cells.size() == totalCubes );

    std::vector<smCube> cubes( smOctreeCell::numberOfSubdivisions );
    cube.subDivide( divisionPerAxis, cubes.data() );

    for ( size_t i = 0; i < smOctreeCell::numberOfSubdivisions; i++ )
    {
        cells[i].cube = cubes[i];
    }
}

const smCube &smOctreeCell::getCube() const
{
    return cube;
}

smCube &smOctreeCell::getCube()
{
    return cube;
}

void smOctreeCell::setCube(const smCube &otherCube)
{
    this->cube.center = otherCube.center;
    this->cube.sideLength = otherCube.sideLength;
}
