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

#include "Core/ContactHandling.h"

#include "Core/CollisionManager.h"

ContactHandling::ContactHandling(const bool typeBilateral)
{
    if (typeBilateral)
    {
        isBilateral = true;
    }
    else
    {
        isBilateral = false;
    }

    type = Unknown;
};

ContactHandling::ContactHandling(const bool typeBilateral,
                                     const std::shared_ptr< SceneObject > first,
                                     const std::shared_ptr< SceneObject > second)
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

    type = Unknown;
}

ContactHandling::~ContactHandling()
{
}

/// \brief Set the scene objects that are colliding
void ContactHandling::setSceneObjects(const std::shared_ptr< SceneObject > first,
                                        const std::shared_ptr< SceneObject > second)
{
    collidingSceneObjects.first = first;
    collidingSceneObjects.second = second;
}

void ContactHandling::setCollisionPairs(const std::shared_ptr< CollisionManager > colPair)
{
    collisionPair = colPair;
}

std::shared_ptr<CollisionManager> ContactHandling::getCollisionPairs() const
{
    return collisionPair;
}

ContactHandling::MethodType ContactHandling::getContactHandlingType() const
{
    return type;
}

std::shared_ptr<SceneObject> ContactHandling::getFirstSceneObject() const
{
	return this->collidingSceneObjects.first;
}

std::shared_ptr<SceneObject> ContactHandling::getSecondSceneObject() const
{
  return this->collidingSceneObjects.second;
}

bool ContactHandling::isUnilateral() const
{
    return !isBilateral;
}
