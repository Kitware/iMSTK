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

#include "OctreeCell.h"

// STD includes
#include <cassert>

// iMSTK includes
#include "Collision/CollisionMoller.h"

namespace imstk {

OctreeCell::OctreeCell() : BaseType()
{

}

OctreeCell::~OctreeCell()
{

}

Vec3d &OctreeCell::getCenter()
{
    return  cube.center;
}

const Vec3d &OctreeCell::getCenter() const
{
    return  cube.center;
}

void OctreeCell::setCenter( const Vec3d &center )
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

bool OctreeCell::isCollidedWithTri(const Vec3d &v0, const Vec3d &v1, const Vec3d &v2 ) const
{
    Eigen::AlignedBox3d box;
    box.min() = cube.leftMinCorner();
    box.max() = cube.rightMaxCorner();

    return (box.contains(v0) || box.contains(v1) || box.contains(v2));
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

}
