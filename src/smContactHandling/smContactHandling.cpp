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

void smContactHandling::setSceneObjects(const std::shared_ptr< smSceneObject >& first,
                        const std::shared_ptr< smSceneObject >& second)
{
    this->collidingSceneObjects.first = first;
    this->collidingSceneObjects.second = second;
}

void smContactHandling::setCollisionPairs(const std::shared_ptr< smCollisionPair >& colPair)
{
    this->collisionPairs = colPair;
}

std::shared_ptr<smCollisionPair> smContactHandling::getCollisionPairs() const
{
    return this->collisionPairs;
}


smContactHandlingType smContactHandling::getContactHandlingType() const
{
    return this->contactHandlingType;
}

std::shared_ptr<smSceneObject> smContactHandling::getFirstSceneObject() const
{ 
	return this->collidingSceneObjects.first; 
}

std::shared_ptr<smSceneObject> smContactHandling::getSecondSceneObject() const
{
    return this->collidingSceneObjects.second; 
}
