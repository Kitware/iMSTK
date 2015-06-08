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

#include "smContactHandling/smContactHandling.h"

smContactHandling::smContactHandling(bool typeBilateral)
{
    if (typeBilateral)
    {
        isBilateral = true;
    }
    else
    {
        isBilateral = false;
    }

    type = SIMMEDTK_CONTACT_UNKNOWN;
};

smContactHandling::smContactHandling(bool typeBilateral,
                                     const std::shared_ptr< smSceneObject >& first,
                                     const std::shared_ptr< smSceneObject >& second)
{
    if (typeBilateral)
    {
        isBilateral = true;
    }
    else
    {
        isBilateral = false;
    }

    setSceneObjects(first, second);

    type = SIMMEDTK_CONTACT_UNKNOWN;
}

smContactHandling::~smContactHandling()
{
}

/// \brief Set the scene objects that are colliding
void smContactHandling::setSceneObjects(
                                        const std::shared_ptr< smSceneObject >& first,
                                        const std::shared_ptr< smSceneObject >& second)
{
    collidingSceneObjects.first = first;
    collidingSceneObjects.second = second;
}

void smContactHandling::setCollisionPairs(const std::shared_ptr< smCollisionPair >& colPair)
{
    collisionPairs = colPair;
}

std::shared_ptr<smCollisionPair> smContactHandling::getCollisionPairs() const
{
    return collisionPairs;
}

smContactHandlingType smContactHandling::getContactHandlingType() const
{
    return type;
}

std::shared_ptr<smSceneObject> smContactHandling::getFirstSceneObject() const
{ 
	return collidingSceneObjects.first; 
}

std::shared_ptr<smSceneObject> smContactHandling::getSecondSceneObject() const
{
    return collidingSceneObjects.second; 
}

bool smContactHandling::isUnilateral()
{
    return !isBilateral;
}