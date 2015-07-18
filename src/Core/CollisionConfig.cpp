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

#include "CollisionConfig.h"

bool smCellTriangle::operator==( unsigned int p_ID )
{
    return ( primID == p_ID ? true : false );
}

bool smCellTriangle::operator==( smCellTriangle &p_tri )
{
    return ( p_tri.primID == primID ? true : false );
}

std::ostream &operator<<( std::ostream &out, smCellTriangle &tri )
{
    out << tri.primID;
    return out;
}

bool smCellLine::operator==( unsigned int p_ID )
{
    return ( primID == p_ID ? true : false );
}

bool smCellLine::operator==( smCellLine &p_line )
{
    return ( p_line.primID == primID ? true : false );
}

std::ostream &operator<<( std::ostream &out, smCellLine &p )
{
    out << p.primID;
    return out;
}

bool smCellModel::operator==( unsigned int p_ID )
{
    return ( primID == p_ID ? true : false );
}
bool smCellModel::operator==( smCellModel &p_model )
{
    return ( p_model.primID == primID ? true : false );
}

std::ostream &operator<<( std::ostream &out, smCellModel &p )
{
    out << p.primID;
    return out;
}

bool smCellPoint::operator==( unsigned int p_ID )
{
    return ( primID == p_ID ? true : false );
}

bool smCellPoint::operator==( smCellPoint &p_point )
{
    return ( p_point.primID == primID ? true : false );
}

std::ostream &operator<<( std::ostream &out, smCellPoint &p )
{
    out << p.primID;
    return out;
}

smCollisionGroup::smCollisionGroup()
{
    groupId = 1;
    groupMask = 1;
}

void smCollisionGroup::setGroup( unsigned int p_groupId )
{
    groupId = 1 << p_groupId;
    groupMask = groupId;
}

void smCollisionGroup::permitCollision( smCollisionGroup &p_group )
{
    groupMask = groupMask | ( p_group.groupId );
    p_group.groupMask = p_group.groupMask | ( groupId );
    std::cout << "Mask:" << groupMask << "\n";
    std::cout << "GroupId:" << groupId << "\n";
}

void smCollisionGroup::forbidCollision( smCollisionGroup &p_group )
{
    groupMask = groupMask & ( ~p_group.groupId );
    p_group.groupMask = p_group.groupMask & ( ~groupId );
}

bool smCollisionGroup::isCollisionPermitted( smCollisionGroup &p_group )
{
    return ( ( groupId & p_group.groupId ) == 0 ? ( ( groupId & p_group.groupMask ) == 0 ? false : true ) : true );
}
