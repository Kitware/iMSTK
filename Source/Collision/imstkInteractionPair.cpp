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

namespace imstk
{
InteractionPair::InteractionPair(std::shared_ptr<CollidingObject> A,
                                 std::shared_ptr<CollidingObject> B,
                                 CollisionDetection::Type         CDType,
                                 CollisionHandling::Type          CHAType,
                                 CollisionHandling::Type          CHBType)
{
    m_colData = std::make_shared<CollisionData>();

    m_valid = false;

    // Check that objects exist
    CHECK(A && B) << "InteractionPair error: invalid objects (nullptr).";

    // Check if objects are different
    /*if (A == B)
    {
        LOG(WARNING) << "InteractionPair error: object cannot interact with itself.";
        return;
    }*/

    // Collision Detection
    std::shared_ptr<CollisionDetection> CD = CollisionDetection::makeCollisionDetectionObject(CDType, A, B, m_colData);

    CHECK(CD) << "InteractionPair error: can not instantiate collision detection algorithm.";

    // Collision Handling A
    std::shared_ptr<CollisionHandling> CHA;
    if (CHAType != CollisionHandling::Type::None)
    {
        CHA = CollisionHandling::make_collision_handling(CHAType, CollisionHandling::Side::A, m_colData, A, B);

        CHECK(CHA) << "InteractionPair error: can not instantiate collision handling for '"
                   << A->getName() << "' object.";
    }

    // Collision Handling B
    std::shared_ptr<CollisionHandling> CHB;
    if (CHBType != CollisionHandling::Type::None)
    {
        CHB = CollisionHandling::make_collision_handling(CHBType, CollisionHandling::Side::B, m_colData, B, A);

        CHECK(CHB) << "InteractionPair error: can not instantiate collision handling for '"
                   << B->getName() << "' object.";
    }

    // Init interactionPair
    m_objects      = ObjectsPair(A, B);
    m_colDetect    = CD;
    m_colHandlingA = CHA;
    m_colHandlingB = CHB;
    m_valid = true;
}

InteractionPair::InteractionPair(std::shared_ptr<CollidingObject>    A,
                                 std::shared_ptr<CollidingObject>    B,
                                 std::shared_ptr<CollisionDetection> CD,
                                 std::shared_ptr<CollisionHandling>  CHA,
                                 std::shared_ptr<CollisionHandling>  CHB)
{
    m_valid = false;

    // Check that objects exist
    CHECK(A && B) << "InteractionPair error: invalid objects (nullptr).";

    // Check if objects are different
    /*if (A == B)
    {
        LOG(WARNING) << "InteractionPair error: object cannot interact with itself.";
        return;
    }*/

    m_objects      = ObjectsPair(A, B);
    m_colDetect    = CD;
    m_colHandlingA = CHA;
    m_colHandlingB = CHB;
    m_colData      = CD->getCollisionData();
    m_valid = true;
}

void
InteractionPair::computeCollisionData()
{
    CHECK(m_valid) << "InteractionPair::computeCollisionData error: interaction not valid.";
    m_colDetect->computeCollisionData();
}

void
InteractionPair::processCollisionData()
{
    CHECK(m_valid) << "InteractionPair::computeContactForces error: interaction not valid.";

    if (m_colHandlingA)
    {
        m_colHandlingA->processCollisionData();
    }
    if (m_colHandlingB)
    {
        m_colHandlingB->processCollisionData();
    }
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

std::shared_ptr<CollisionDetection>
InteractionPair::getCollisionDetection() const
{
    return m_colDetect;
}

std::shared_ptr<CollisionHandling>
InteractionPair::getCollisionHandlingA() const
{
    return m_colHandlingA;
}

std::shared_ptr<CollisionHandling>
InteractionPair::getCollisionHandlingB() const
{
    return m_colHandlingB;
}
}
