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

#ifndef SM_OCTREECELL_H
#define SM_OCTREECELL_H

// SimMedTK includes
#include "smCollision/smSurfaceTreeCell.h"
#include "smCore/smGeometry.h"

/// \brief cell of an octree
class smOctreeCell : public smSurfaceTreeCell<smOctreeCell>
{
protected:
    using BaseType = smSurfaceTreeCell<smOctreeCell>;
    using SelfType = smOctreeCell;

public:
    smOctreeCell();
    ~smOctreeCell();

    enum { numberOfSubdivisions = 8 };

    /// \brief get a constant reference to the cell primitive
    const smCube &getCube() const;

    /// \brief get a reference to the cell primitive
    smCube &getCube();

    /// \brief set the primitive
    void setCube(const smCube &other);

    /// \brief get the center of the octree cell
    smVec3f &getCenter();

    /// \brief get constant reference to the center of the octree cell
    const smVec3f &getCenter() const;

    /// \brief set the center of the octree cell
    void setCenter ( const smVec3f &center );

    /// \brief get reference to the side length of the octree cell
    float &getLength ();

    /// \brief get the side length of the octree cell
    const float &getLength() const;

    /// \brief set the octree cell
    void copyShape ( const smOctreeCell &cell );

    /// \brief !! expand the cell of the octree structure
    void expand ( const float expandScale );

    /// \brief set the length of the octree cell
    void setLength ( const float length );

    /// \brief check if a triangle is intersecting the octree cell
    bool isCollidedWithTri ( smVec3f &v0, smVec3f &v1, smVec3f &v2 );

    /// \brief check if a point lies inside an octree cell
    bool isCollidedWithPoint ();

    /// \brief subdivide the cells of octree cells
    void subDivide ( const int divisionPerAxis, std::vector<smOctreeCell> &cells );

private:
    smCube cube; ///< cube
};

#endif

