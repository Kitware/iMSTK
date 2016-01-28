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

#ifndef CORE_CONTACTHANDLING_H
#define CORE_CONTACTHANDLING_H

// STL includes
#include <memory>
#include <unordered_map>

// iMSTK includes
#include "Core/CoreClass.h"
#include "Core/Vector.h"

namespace imstk {

class Mesh;
class CollisionManager;
class InteractionSceneModel;


/// \brief Base class to for handling contact for collision response
/// Input: collision pairs containing the collision information
/// Output: contact forces or Jacobians or projection/PBD constraints
class ContactHandling: public CoreClass
{
public:
    /// \brief class types.
    /// Each contact handling algorithm should have type of itself
    enum MethodType
    {
        Penalty=1,
        Lcp,
        PenaltyFemToStatic,
        NoContact,
        Unknown=-1
    };

public:

    ContactHandling(const bool typeBilateral);

    ContactHandling(const bool typeBilateral,
                      const std::shared_ptr<InteractionSceneModel> sceneObjFirst,
                      const std::shared_ptr<InteractionSceneModel> sceneObjSecond);

    virtual ~ContactHandling();

    /// \brief returns true if bilateral
    bool isUnilateral() const;

    /// \brief Set the scene objects that are colliding
    void setInteractionSceneModels(const std::shared_ptr< InteractionSceneModel > first,
                         const std::shared_ptr< InteractionSceneModel > second);

    /// \brief Set the collision pair data structure in which the information
    /// needs to be stored
    void setCollisionPairs(const std::shared_ptr<CollisionManager> colPair);

    /// \brief Get the colliison information contained in the collision pairs
    std::shared_ptr<CollisionManager> getCollisionPairs() const;

    /// \brief Get if the contact handling is unilateral or bilateral
    MethodType getContactHandlingType() const;

    /// \brief Get the first scene object
    std::shared_ptr<InteractionSceneModel> getFirstInteractionSceneModel() const;

    /// \brief Get the second scene object
    std::shared_ptr<InteractionSceneModel> getSecondInteractionSceneModel() const;

    /// \brief Implementation of how the contacts between colliding
    /// objects is resolved
    virtual void resolveContacts() = 0;

protected:

    MethodType type;

    bool isBilateral;

    std::pair<std::shared_ptr<InteractionSceneModel>, std::shared_ptr<InteractionSceneModel>> collidingInteractionSceneModels;

    std::shared_ptr<CollisionManager> collisionPair;
};

}

#endif //CORE_CONTACTHANDLING_H
