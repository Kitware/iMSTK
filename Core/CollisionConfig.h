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

#ifndef SMCOLLISIONCONFIG_H
#define SMCOLLISIONCONFIG_H

// SimMedTK includes
#include "Core/Config.h"
#include "Core/Vector.h"
#include "Core/CoreClass.h"

struct GeometryRepresentation {};

/// \brief Contains information related to the triangles that are present in a cell
/// at any given point
struct CellTriangle : public GeometryRepresentation
{
    CellTriangle() : primID(0) {}
    CellTriangle(const unsigned int &id) : primID(id) {}
    unsigned int primID;
    std::shared_ptr<UnifiedId> meshID;
    core::Vec3d vert[3];

    bool operator ==(unsigned int p_ID);

    bool operator ==(CellTriangle &p_tri);

    friend std::ostream &operator<<(std::ostream &out, CellTriangle &tri);
};

/// \brief Contains information related to the a line segments that are present in a cell
/// at any given point
struct CellLine : public GeometryRepresentation
{
    CellLine() : primID(0) {}
    CellLine(const unsigned int &id) : primID(id) {}
    unsigned int primID; ///< Edge id
    std::shared_ptr<UnifiedId> meshID; ///< MeshLine id
    core::Vec3d vert[2]; ///< Vertices

    bool operator ==(unsigned int p_ID);
    bool operator ==(CellLine &p_line);

    friend std::ostream &operator<<(std::ostream &out, CellLine &p);
};

/// \brief !!
struct CellModel : public GeometryRepresentation
{
    CellModel() : primID(0) {}
    CellModel(const unsigned int &id) : primID(id) {}
    unsigned int primID; ///< Model Prim id
    std::shared_ptr<UnifiedId> meshID; ///< MeshLine id
    core::Vec3d center; ///< Vertices
    double radius;

    bool operator ==(unsigned int p_ID);

    bool operator ==(CellModel &p_model);

    friend std::ostream &operator<<(std::ostream &out, CellModel &p);
};


/// \brief Contains information related to the a verticess that are present in a cell
/// at any given point
struct CellPoint : public GeometryRepresentation
{
    CellPoint() : primID(0) {}
    CellPoint(const unsigned int &id) : primID(id) {}
    unsigned int primID; ///< Model Prim id
    std::shared_ptr<UnifiedId> meshID; ///< MeshLine id
    core::Vec3d vert; ///< Vertices

    bool operator ==(unsigned int p_ID);

    bool operator ==(CellPoint &p_point);

    friend std::ostream &operator<<(std::ostream &out, CellPoint &p);
};


/// \brief Contains triangle pair that have collided
struct CollidedTriangles
{
    CellTriangle tri1;
    CellTriangle tri2;
    core::Vec3d proj1, proj2;
    short point1, point2;
};

/// \brief Contains line pair that have collided
struct CollidedLineTris
{
    CellTriangle tri;
    CellLine line;
    core::Vec3d intersection;
};

/// \brief Contains model-point pair that have collided
struct CollidedModelPoints
{
    CellModel model;
    CellPoint point;
    double penetration;
};

/// \brief !!
class CollisionGroup
{
public:

    size_t groupId;
    size_t groupMask;

    /// \brief !!
    CollisionGroup();

    /// \brief !!
    void setGroup(unsigned int p_groupId);

    /// \brief !!
    void permitCollision(CollisionGroup &p_group);

    /// \brief !!
    void forbidCollision(CollisionGroup &p_group);

    /// \brief if the group is different then the function checks the group mask..
    //  if they're in the same group collision enabled
    bool isCollisionPermitted(CollisionGroup& p_group);

};

#endif
