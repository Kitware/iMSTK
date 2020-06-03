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
#include "imstkLogger.h"
#include "imstkObjectInteractionPair.h"
#include "imstkSceneObject.h"

namespace imstk
{
//CollisionGraph::ObjectInteractionPtr
//CollisionGraph::addInteractionPair(SceneObjectPtr       A,
//    SceneObjectPtr       B,
//                                   CollisionDetection::Type CDType,
//                                   CollisionHandling::Type  CHAType,
//                                   CollisionHandling::Type  CHBType)
//{
//    // Check that interaction pair does not exist
//    //if (this->getInteractionPair(A, B) != nullptr)
//    //{
//    //    LOG(WARNING) << "CollisionGraph::addInteractionPair error: interaction already defined for "
//    //                 << A->getName() << " & " << B->getName() << ".";
//    //    return nullptr;
//    //}
//
//    //// Create interaction pair
//    //auto intPair = std::make_shared<CollisionPair>(A, B, CDType, CHAType, CHBType);
//
//    //// Check validity
//    ///*if (!intPair->isValid())
//    //{
//    //    LOG(WARNING) << "CollisionGraph::addInteractionPair error: could not create interaction for "
//    //                 << A->getName() << " & " << B->getName() << " with those parameters.";
//    //    intPair.reset();
//    //    return nullptr;
//    //}*/
//
//    //// Populate book-keeping
//    //m_interactionPairs.push_back(intPair);
//    //m_interactionPairMap[A].push_back(intPair);
//    //m_interactionPairMap[B].push_back(intPair);
//
//    //// Return interaction pair
//    //return intPair;
//    return nullptr;
//}
//
//CollisionGraph::ObjectInteractionPtr
//CollisionGraph::addInteractionPair(SceneObjectPtr    A,
//    SceneObjectPtr    B,
//                                   CollisionDetectionPtr CD,
//                                   CollisionHandlingPtr  CHA,
//                                   CollisionHandlingPtr  CHB)
//{
//    //// Check that interaction pair does not exist
//    //if (this->getInteractionPair(A, B) != nullptr)
//    //{
//    //    LOG(WARNING) << "CollisionGraph::addInteractionPair error: interaction already defined for "
//    //                 << A->getName() << " & " << B->getName() << ".";
//    //    return nullptr;
//    //}
//
//    //// Create interaction pair
//    //auto intPair = std::make_shared<CollisionPair>(A, B, CD, CHA, CHB);
//
//    //// Check validity
//    ///*if (!intPair->isValid())
//    //{
//    //    LOG(WARNING) << "CollisionGraph::addInteractionPair error: could not create interaction for "
//    //                 << A->getName() << " & " << B->getName() << " with those parameters.";
//    //    intPair.reset();
//    //    return nullptr;
//    //}*/
//
//    //// Populate book-keeping
//    //m_interactionPairs.push_back(intPair);
//    //m_interactionPairMap[A].push_back(intPair);
//    //m_interactionPairMap[B].push_back(intPair);
//
//    //// Return interaction pair
//    //return intPair;
//    return nullptr;
//}

void
CollisionGraph::addInteraction(ObjectInteractionPtr pair)
{
    m_interactionPairs.push_back(pair);
    m_interactionPairMap[pair->getObjectsPair().first].push_back(pair);
    m_interactionPairMap[pair->getObjectsPair().second].push_back(pair);
}

bool
CollisionGraph::removeInteractionPair(SceneObjectPtr A, SceneObjectPtr B)
{
    //std::shared_ptr<InteractionPair> intPair = this->getInteractionPair(A, B);

    //// Check that interaction pair exists
    //if (intPair == nullptr)
    //{
    //    LOG(WARNING) << "CollisionGraph::removeInteractionPair error: no such pair for objects "
    //                 << A->getName() << " & " << B->getName() << ".";
    //    return false;
    //}

    //// Remove interaction pair from list
    //auto it = std::remove(m_interactionPairs.begin(),
    //    m_interactionPairs.end(),
    //    intPair);
    //m_interactionPairs.erase(it, m_interactionPairs.end());

    //// Remove interaction pair from maps
    //it = std::remove(m_interactionPairMap.at(A).begin(),
    //                 m_interactionPairMap.at(A).end(),
    //                 intPair);
    //m_interactionPairMap.at(A).erase(it, m_interactionPairMap.at(A).end());

    //it = std::remove(m_interactionPairMap.at(B).begin(),
    //                 m_interactionPairMap.at(B).end(),
    //                 intPair);
    //m_interactionPairMap.at(B).erase(it, m_interactionPairMap.at(B).end());

    //// If no more interactions for objects, remove objects from list
    //if (m_interactionPairMap.at(A).empty())
    //{
    //    m_interactionPairMap.erase(A);
    //}
    //if (m_interactionPairMap.at(B).empty())
    //{
    //    m_interactionPairMap.erase(B);
    //}

    return true;
}

bool
CollisionGraph::removeInteractionPair(ObjectInteractionPtr intPair)
{
    /* std::shared_ptr<CollidingObject> obj1 = std::static_pointer_cast<CollidingObject>(intPair->getObjectsPair().first);
     std::shared_ptr<CollidingObject> obj2 = std::static_pointer_cast<CollidingObject>(intPair->getObjectsPair().second);
     return this->removeInteractionPair(obj1, obj2);*/
    return true;
}

CollisionGraph::ObjectInteractionPtr
CollisionGraph::getInteractionPair(SceneObjectPtr A, SceneObjectPtr B)
{
    /*for (const auto& intPair : m_interactionPairs)
    {
        if (intPair->getObjectsPair() == std::pair<std::shared_ptr<SceneObject>, std::shared_ptr<SceneObject>>(A, B))
        {
            return intPair;
        }
    }*/
    return nullptr;
}

const std::unordered_map<CollisionGraph::SceneObjectPtr, std::vector<CollisionGraph::ObjectInteractionPtr>>&
CollisionGraph::getInteractionPairMap() const
{
    return m_interactionPairMap;
}
} // imstk
