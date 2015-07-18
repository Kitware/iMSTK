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
#include "Config.h"
#include "Vector.h"
#include "CoreClass.h"

struct GeometryRepresentation {};

/// \brief Contains information related to the triangles that are present in a cell
/// at any given point
struct smCellTriangle : public GeometryRepresentation
{
    smCellTriangle() : primID(0) {}
    smCellTriangle(const smUInt &id) : primID(id) {}
    smUInt primID;
    std::shared_ptr<smUnifiedId> meshID;
    smVec3d vert[3];

    bool operator ==(unsigned int p_ID);

    bool operator ==(smCellTriangle &p_tri);

    friend std::ostream &operator<<(std::ostream &out, smCellTriangle &tri);
};

/// \brief Contains information related to the a line segments that are present in a cell
/// at any given point
struct smCellLine : public GeometryRepresentation
{
    smCellLine() : primID(0) {}
    smCellLine(const smUInt &id) : primID(id) {}
    smUInt primID; ///< Edge id
    std::shared_ptr<smUnifiedId> meshID; ///< smMeshLine id
    smVec3d vert[2]; ///< Vertices

    bool operator ==(unsigned int p_ID);
    bool operator ==(smCellLine &p_line);

    friend std::ostream &operator<<(std::ostream &out, smCellLine &p);
};

/// \brief !!
struct smCellModel : public GeometryRepresentation
{
    smCellModel() : primID(0) {}
    smCellModel(const smUInt &id) : primID(id) {}
    smUInt primID; ///< Model Prim id
    std::shared_ptr<smUnifiedId> meshID; ///< smMeshLine id
    smVec3d center; ///< Vertices
    double radius;

    bool operator ==(unsigned int p_ID);

    bool operator ==(smCellModel &p_model);

    friend std::ostream &operator<<(std::ostream &out, smCellModel &p);
};


/// \brief Contains information related to the a verticess that are present in a cell
/// at any given point
struct smCellPoint : public GeometryRepresentation
{
    smCellPoint() : primID(0) {}
    smCellPoint(const smUInt &id) : primID(id) {}
    smUInt primID; ///< Model Prim id
    std::shared_ptr<smUnifiedId> meshID; ///< smMeshLine id
    smVec3d vert; ///< Vertices

    bool operator ==(unsigned int p_ID);

    bool operator ==(smCellPoint &p_point);

    friend std::ostream &operator<<(std::ostream &out, smCellPoint &p);
};


/// \brief Contains triangle pair that have collided
struct smCollidedTriangles
{
    smCellTriangle tri1;
    smCellTriangle tri2;
    smVec3d proj1, proj2;
    smShort point1, point2;
};

/// \brief Contains line pair that have collided
struct smCollidedLineTris
{
    smCellTriangle tri;
    smCellLine line;
    smVec3d intersection;
};

/// \brief Contains model-point pair that have collided
struct smCollidedModelPoints
{
    smCellModel model;
    smCellPoint point;
    double penetration;
};

/// \brief !!
class smCollisionGroup
{
public:

    smUInt64  groupId;
    smUInt64  groupMask;

    /// \brief !!
    smCollisionGroup();

    /// \brief !!
    void setGroup(smUInt p_groupId);

    /// \brief !!
    void permitCollision(smCollisionGroup &p_group);

    /// \brief !!
    void forbidCollision(smCollisionGroup &p_group);

    /// \brief if the group is different then the function checks the group mask..
    //  if they're in the same group collision enabled
    smBool isCollisionPermitted(smCollisionGroup& p_group);

};

#endif
