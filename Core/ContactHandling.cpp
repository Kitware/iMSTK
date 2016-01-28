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

#include "Core/ContactHandling.h"

#include "Core/CollisionManager.h"
#include "SceneModels/InteractionSceneModel.h"

namespace imstk {

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

//---------------------------------------------------------------------------
ContactHandling::ContactHandling(const bool typeBilateral,
                                     const std::shared_ptr< InteractionSceneModel > first,
                                     const std::shared_ptr< InteractionSceneModel > second)
{
    if (typeBilateral)
    {
        isBilateral = true;
    }
    else
    {
        isBilateral = false;
    }

    setInteractionSceneModels(first, second);

    type = Unknown;
}

//---------------------------------------------------------------------------
ContactHandling::~ContactHandling()
{
}

/// \brief Set the scene objects that are colliding
void ContactHandling::setInteractionSceneModels(const std::shared_ptr< InteractionSceneModel > first,
                                        const std::shared_ptr< InteractionSceneModel > second)
{
    collidingInteractionSceneModels.first = first;
    collidingInteractionSceneModels.second = second;
}

//---------------------------------------------------------------------------
void ContactHandling::setCollisionPairs(const std::shared_ptr< CollisionManager > colPair)
{
    collisionPair = colPair;
}

//---------------------------------------------------------------------------
std::shared_ptr<CollisionManager> ContactHandling::getCollisionPairs() const
{
    return collisionPair;
}

//---------------------------------------------------------------------------
ContactHandling::MethodType ContactHandling::getContactHandlingType() const
{
    return type;
}

//---------------------------------------------------------------------------
std::shared_ptr<InteractionSceneModel> ContactHandling::getFirstInteractionSceneModel() const
{
	return this->collidingInteractionSceneModels.first;
}

//---------------------------------------------------------------------------
std::shared_ptr<InteractionSceneModel> ContactHandling::getSecondInteractionSceneModel() const
{
  return this->collidingInteractionSceneModels.second;
}

//---------------------------------------------------------------------------
bool ContactHandling::isUnilateral() const
{
    return !isBilateral;
}

}
