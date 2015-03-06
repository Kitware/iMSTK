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

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smCore/smCoreClass.h"

/// \brief Contains information related to the triangles that are present in a cell
/// at any given point
struct smCellTriangle
{

    smUInt primID;
    smUnifiedID meshID;
    smVec3<smFloat> vert[3];

    inline bool operator ==(unsigned int p_ID)
    {
        return (primID == p_ID ? true : false);
    }

    inline  bool operator ==(smCellTriangle &p_tri)
    {
        return (p_tri.primID == primID ? true : false);
    }

    friend std::ostream &operator<<(std::ostream &out, smCellTriangle &tri)
    {
        out << tri.primID;
        return out;
    }
};

/// \brief Contains information related to the a line segments that are present in a cell
/// at any given point
struct smCellLine
{

    smUInt primID; ///< Edge id
    smUnifiedID meshID; ///< smMeshLine id
    smVec3<smFloat> vert[2]; ///< Vertices

    inline  bool operator ==(unsigned int p_ID)
    {
        return (primID == p_ID ? true : false);
    }
    inline  bool operator ==(smCellLine &p_line)
    {
        return (p_line.primID == primID ? true : false);
    }

    friend std::ostream &operator<<(std::ostream &out, smCellLine &p)
    {
        out << p.primID;
        return out;
    }
};

/// \brief !!
struct smCellModel
{

    smUInt primID; ///< Model Prim id
    smUnifiedID meshID; ///< smMeshLine id
    smVec3<smFloat> center; ///< Vertices
    smFloat radius;

    inline  bool operator ==(unsigned int p_ID)
    {
        return (primID == p_ID ? true : false);
    }

    inline  bool operator ==(smCellModel &p_model)
    {
        return (p_model.primID == primID ? true : false);
    }

    friend std::ostream &operator<<(std::ostream &out, smCellModel &p)
    {
        out << p.primID;
        return out;
    }
};


/// \brief Contains information related to the a verticess that are present in a cell
/// at any given point
struct smCellPoint
{

    smUInt primID; ///< Model Prim id
    smUnifiedID meshID; ///< smMeshLine id
    smVec3<smFloat> vert; ///< Vertices

    inline  bool operator ==(unsigned int p_ID)
    {
        return (primID == p_ID ? true : false);
    }

    inline  bool operator ==(smCellPoint &p_point)
    {
        return (p_point.primID == primID ? true : false);
    }

    friend std::ostream &operator<<(std::ostream &out, smCellPoint &p)
    {
        out << p.primID;
        return out;
    }
};

/// \brief Contains triangle pair that have collided
struct smCollidedTriangles
{
    smCellTriangle tri1;
    smCellTriangle tri2;
    smVec3<smFloat> proj1, proj2;
    smShort point1, point2;
};

/// \brief Contains line pair that have collided
struct smCollidedLineTris
{
    smCellTriangle tri;
    smCellLine line;
    smVec3<smFloat> intersection;
};

/// \brief Contains model-point pair that have collided
struct smCollidedModelPoints
{
    smCellModel model;
    smCellPoint point;
    smFloat penetration;
};

/// \brief !!
class smCollisionGroup
{

protected:

public:

    smUInt64  groupId;
    smUInt64  groupMask;

    /// \brief !!
    smCollisionGroup()
    {
        groupId = 1;
        groupMask = 1;
    }

    /// \brief !!
    void setGroup(smUInt p_groupId)
    {
        groupId = 1 << p_groupId;
        groupMask = groupId;
    }

    /// \brief !!
    void permitCollision(smCollisionGroup &p_group)
    {
        groupMask = groupMask | (p_group.groupId);
        p_group.groupMask = p_group.groupMask | (groupId);
        std::cout << "Mask:" << groupMask << "\n";
        std::cout << "GroupId:" << groupId << "\n";
    }

    /// \brief !!
    void forbidCollision(smCollisionGroup &p_group)
    {
        groupMask = groupMask & (~p_group.groupId);
        p_group.groupMask = p_group.groupMask & (~groupId);
    }

    /// \brief if the group is different then the function checks the group mask..
    //  if they're in the same group collision enabled
    inline smBool isCollisionPermitted(smCollisionGroup& p_group)
    {
        return ((groupId & p_group.groupId) == 0 ? ((groupId & p_group.groupMask) == 0 ? false : true) : true);
    }

};

#endif
