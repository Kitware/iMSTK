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

#ifndef SMCONTACTHANDLING_H
#define SMCONTACTHANDLING_H

// SimMedTK includes
#include "CoreClass.h"
#include "SceneObject.h"

// STL includes
#include <memory>

class smMesh;
class CollisionPair;

enum class ContactHandlingType
{
    PenaltyFemToStatic,
    UNKNOWN
};

/// \brief Base class to for handling contact for collision response
/// Input: collision pairs containing the collision information
/// Output: contact forces or Jacobians or projection/PBD constraints
class ContactHandling: public CoreClass
{
public:

    ContactHandling(const bool typeBilateral);

    ContactHandling(const bool typeBilateral,
                      const std::shared_ptr<SceneObject> sceneObjFirst,
                      const std::shared_ptr<SceneObject> sceneObjSecond);

    virtual ~ContactHandling();

    /// \brief returns true if bilateral
    bool isUnilateral() const;

    /// \brief Set the scene objects that are colliding
    void setSceneObjects(const std::shared_ptr< SceneObject > first,
                         const std::shared_ptr< SceneObject > second);

    /// \brief Set the collision pair data structure in which the information
    /// needs to be stored
    void setCollisionPairs(const std::shared_ptr< CollisionPair > colPair);

    /// \brief Get the colliison information contained in the collision pairs
    std::shared_ptr<CollisionPair> getCollisionPairs() const;

    /// \brief Get if the contact handling is unilateral or bilateral
    ContactHandlingType getContactHandlingType() const;

    /// \brief Get the first scene object
    std::shared_ptr<SceneObject> getFirstSceneObject() const;

    /// \brief Get the second scene object
    std::shared_ptr<SceneObject> getSecondSceneObject() const;

    /// \brief Implementation of how the contacts between colliding
    /// objects is resolved
    virtual void resolveContacts() = 0;
protected:

    ContactHandlingType type;

    bool isBilateral;

    std::pair<std::shared_ptr<SceneObject>, std::shared_ptr<SceneObject>> collidingSceneObjects;

    std::shared_ptr<CollisionPair> collisionPairs;
};

#endif //SMCONTACTHANDLING_H
