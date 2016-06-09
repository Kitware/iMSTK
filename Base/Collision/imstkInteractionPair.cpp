/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkInteractionPair.h"

#include <g3log/g3log.hpp>

namespace imstk {

InteractionPair::InteractionPair(std::shared_ptr<CollidingObject> A,
                                 std::shared_ptr<CollidingObject> B,
                                 CollisionDetection::Type CDType,
                                 CollisionHandling::Type CHAType,
                                 CollisionHandling::Type CHBType)
{
    m_valid = false;

    // Check that objects exist
    if (A == nullptr || B == nullptr)
    {
        LOG(WARNING) << "InteractionPair error: invalid objects (nullptr).";
        return;
    }

    // Check if objects are differents
    if (A == B)
    {
        LOG(WARNING) << "InteractionPair error: object cannot interact with itself.";
        return;
    }

    // Check that at least one object is not static
    if (A->getType() == SceneObject::Type::Static &&
        B->getType() == SceneObject::Type::Static)
    {
       LOG(WARNING) << "InteractionPair error: can not create interaction between two static objects.";
       //return;
    }

    // Collision Detection
    m_colDataMap[A] = CollisionData();
    m_colDataMap[B] = CollisionData();
    std::shared_ptr<CollisionDetection> CD = CollisionDetection::make_collision_detection(CDType, A, B,
                                                                                          m_colDataMap[A], m_colDataMap[B]);
    if (CD == nullptr)
    {
        LOG(WARNING) << "InteractionPair error: can not instantiate collision detection algorithm.";
        return;
    }

    // Collision Handling A
    std::shared_ptr<CollisionHandling> CHA;
    if (A->getType() != SceneObject::Type::Static)
    {
       CHA = CollisionHandling::make_collision_handling(CHAType, A, m_colDataMap[A], B);
       if (CHA == nullptr)
       {
           LOG(WARNING) << "InteractionPair error: can not instantiate collision handling for '"
                        << A->getName() << "' object.";
           return;
       }
    }

    // Collision Handling B
    std::shared_ptr<CollisionHandling> CHB;
    if (B->getType() != SceneObject::Type::Static)
    {
       CHB = CollisionHandling::make_collision_handling(CHBType, B, m_colDataMap[B], A);
       if (CHB == nullptr)
       {
           LOG(WARNING) << "InteractionPair error: can not instantiate collision handling for '"
                        << B->getName() << "' object.";
           return;
       }
    }

    // Init interactionPair
    m_objects = ObjectsPair(A, B);
    m_colDetect = CD;
    m_colHandlingMap[A] = CHA;
    m_colHandlingMap[B] = CHB;
    m_valid = true;
}

void
InteractionPair::computeCollisionData()
{
    if (!m_valid)
    {
        LOG(WARNING) << "InteractionPair::computeCollisionData error: interaction not valid.";
        return;
    }

    m_colDetect->computeCollisionData();
}

void
InteractionPair::computeContactForces(std::shared_ptr<CollidingObject> obj)
{
    if (!m_valid)
    {
        LOG(WARNING) << "InteractionPair::computeContactForces error: interaction not valid.";
        return;
    }

    if (obj != m_objects.first && obj != m_objects.second)
    {
        LOG(WARNING) << "InteractionPair::computeContactForces error: "
                     << obj->getName() << " is not part of this interaction Pair.";
        return;
    }

    if (obj->getType() == SceneObject::Type::Static)
    {
        return;
    }

    m_colHandlingMap.at(obj)->computeContactForces();
}

const bool&
InteractionPair::isValid()
{
    return m_valid;
}

const InteractionPair::ObjectsPair&
InteractionPair::getObjectsPair() const
{
    return m_objects;
}

}
