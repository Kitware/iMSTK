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

#include "imstkCollisionGraph.h"

#include <g3log/g3log.hpp>

namespace imstk
{

std::shared_ptr<InteractionPair>
CollisionGraph::addInteractionPair(CollidingObjectPtr A,
                                   CollidingObjectPtr B,
                                   CollisionDetection::Type CDType,
                                   CollisionHandling::Type CHAType,
                                   CollisionHandling::Type CHBType)
{
    // Check that interaction pair does not exist
    if (this->getInteractionPair(A,B) != nullptr)
    {
        LOG(WARNING) << "CollisionGraph::addInteractionPair error: interaction already defined for "
                     << A->getName() << " & " << B->getName() << ".";
        return nullptr;
    }

    // Create interaction pair
    auto intPair = std::make_shared<InteractionPair>(A, B, CDType, CHAType, CHBType);

    // Check validity
    if (!intPair->isValid())
    {
        LOG(WARNING) << "CollisionGraph::addInteractionPair error: could not create interaction for "
                     << A->getName() << " & " << B->getName() << " with those parameters.";
        intPair.reset();
        return nullptr;
    }

    // Populate book-keeping
    m_interactionPairList.push_back(intPair);
    m_interactionPairMap[A].push_back(intPair);
    m_interactionPairMap[B].push_back(intPair);

    // Return interaction pair
    return intPair;
}

void CollisionGraph::addInteractionPair(std::shared_ptr<PbdInteractionPair> pair)
{
    m_interactionPbdPairList.push_back(pair);
}

bool
CollisionGraph::removeInteractionPair(CollidingObjectPtr A, CollidingObjectPtr B)
{
    std::shared_ptr<InteractionPair> intPair = this->getInteractionPair(A,B);

    // Check that interaction pair exists
    if (intPair == nullptr)
    {
        LOG(WARNING) << "CollisionGraph::removeInteractionPair error: no such pair for objects "
                     << A->getName() << " & " << B->getName() << ".";
        return false;
    }

    // Remove interaction pair from list
    auto it = std::remove(m_interactionPairList.begin(),
                          m_interactionPairList.end(),
                          intPair);
    m_interactionPairList.erase(it, m_interactionPairList.end());

    // Remove interaction pair from maps
    it = std::remove(m_interactionPairMap.at(A).begin(),
                     m_interactionPairMap.at(A).end(),
                     intPair);
    m_interactionPairMap.at(A).erase(it, m_interactionPairMap.at(A).end());

    it = std::remove(m_interactionPairMap.at(B).begin(),
                     m_interactionPairMap.at(B).end(),
                     intPair);
    m_interactionPairMap.at(B).erase(it, m_interactionPairMap.at(B).end());

    // If no more interactions for objects, remove objects from list
    if (m_interactionPairMap.at(A).empty())
    {
        m_interactionPairMap.erase(A);
    }
    if (m_interactionPairMap.at(B).empty())
    {
        m_interactionPairMap.erase(B);
    }

    return true;
}

bool
CollisionGraph::removeInteractionPair(InteractionPairPtr intPair)
{
    return this->removeInteractionPair(intPair->getObjectsPair().first,
                                       intPair->getObjectsPair().second);
}

const std::vector<std::shared_ptr<PbdInteractionPair> > &CollisionGraph::getPbdPairList() const
{
    return m_interactionPbdPairList;
}

std::shared_ptr<InteractionPair>
CollisionGraph::getInteractionPair(CollidingObjectPtr A, CollidingObjectPtr B)
{
    for (const auto& intPair : m_interactionPairList)
    {
        if (intPair->getObjectsPair() == std::pair<CollidingObjectPtr,CollidingObjectPtr>(A,B) ||
            intPair->getObjectsPair() == std::pair<CollidingObjectPtr,CollidingObjectPtr>(B,A))
        {
            return intPair;
        }
    }
    return nullptr;
}

const std::vector<std::shared_ptr<InteractionPair>>&
CollisionGraph::getInteractionPairList() const
{
    return m_interactionPairList;
}

const std::unordered_map<std::shared_ptr<CollidingObject>, std::vector<std::shared_ptr<InteractionPair>>>&
CollisionGraph::getInteractionPairMap() const
{
    return m_interactionPairMap;
}

}
