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

#include "OctreeCell.h"

// STD includes
#include <cassert>

// SimMedTK includes
#include "Collision/CollisionMoller.h"

OctreeCell::OctreeCell() : BaseType()
{

}

OctreeCell::~OctreeCell()
{

}

core::Vec3d &OctreeCell::getCenter()
{
    return  cube.center;
}

const core::Vec3d &OctreeCell::getCenter() const
{
    return  cube.center;
}

void OctreeCell::setCenter( const core::Vec3d &center )
{
    cube.center = center;
}

double &OctreeCell::getLength()
{
    return cube.sideLength;
}

const double &OctreeCell::getLength() const
{
    return cube.sideLength;
}

void OctreeCell::setLength( const double length )
{
    cube.sideLength = length;
}

void OctreeCell::copyShape( const OctreeCell &cell )
{
    cube = cell.cube;
}

void OctreeCell::expand( const double expandScale )
{
    cube.expand( expandScale );
}

bool OctreeCell::isCollidedWithTri( core::Vec3d &v0, core::Vec3d &v1, core::Vec3d &v2 )
{
    AABB tempAABB;
    tempAABB.aabbMin = cube.leftMinCorner();
    tempAABB.aabbMax = cube.rightMaxCorner();
    return CollisionMoller::checkAABBTriangle( tempAABB, v0, v1, v2 );
}

bool OctreeCell::isCollidedWithPoint()
{
    std::cerr << "Error::OctreeCell::isCollidedWithPoint(): Function not implemented." << std::endl;
    return 0;
}

void OctreeCell::subDivide( const int divisionPerAxis,
                              std::array<OctreeCell, OctreeCell::numberOfSubdivisions> &cells )
{
    size_t totalCubes = divisionPerAxis * divisionPerAxis * divisionPerAxis;

    assert( cells.size() == totalCubes );

    std::vector<Cube> cubes( OctreeCell::numberOfSubdivisions );
    cube.subDivide( divisionPerAxis, cubes.data() );

    for ( size_t i = 0; i < OctreeCell::numberOfSubdivisions; i++ )
    {
        cells[i].cube = cubes[i];
    }
}

const Cube &OctreeCell::getCube() const
{
    return cube;
}

Cube &OctreeCell::getCube()
{
    return cube;
}

void OctreeCell::setCube(const Cube &otherCube)
{
    this->cube.center = otherCube.center;
    this->cube.sideLength = otherCube.sideLength;
}
