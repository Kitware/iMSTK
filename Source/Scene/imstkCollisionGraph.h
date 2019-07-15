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

#pragma once

#include <vector>
#include <unordered_map>

// imstk
#include "imstkCollidingObject.h"
#include "imstkInteractionPair.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionHandling.h"

#include "imstkPbdInteractionPair.h"

namespace imstk
{
///
/// \class CollisionGraph
///
/// \brief
///
class CollisionGraph
{
public:
    using CollidingObjectPtr    = std::shared_ptr<CollidingObject>;
    using CollisionHandlingPtr  = std::shared_ptr<CollisionHandling>;
    using CollisionDetectionPtr = std::shared_ptr<CollisionDetection>;
    using InteractionPairPtr    = std::shared_ptr<InteractionPair>;

    ///
    /// \brief Default constructor
    ///
    CollisionGraph() = default;

    ///
    /// \brief Default destructor
    ///
    ~CollisionGraph() = default;

    ///
    /// \brief Add interaction pair in collision graph
    ///
    InteractionPairPtr addInteractionPair(CollidingObjectPtr       A,
                                          CollidingObjectPtr       B,
                                          CollisionDetection::Type CDType,
                                          CollisionHandling::Type  CHAType,
                                          CollisionHandling::Type  CHBType);

    //TODO: Refactor -> PBD only
    InteractionPairPtr addInteractionPair(CollidingObjectPtr    A,
                                          CollidingObjectPtr    B,
                                          CollisionDetectionPtr CD,
                                          CollisionHandlingPtr  CHA,
                                          CollisionHandlingPtr  CHB);

    void addInteractionPair(std::shared_ptr<PbdInteractionPair> pair);

    ///
    /// \brief Remove interaction pair in collision graph
    ///
    bool removeInteractionPair(CollidingObjectPtr A, CollidingObjectPtr B);
    bool removeInteractionPair(InteractionPairPtr intPair);

    ///
    /// \brief Returns the interaction pair if it exists
    ///
    InteractionPairPtr getInteractionPair(CollidingObjectPtr A, CollidingObjectPtr B);

    ///
    /// \brief Returns a vector of all interaction pairs in the collision graph
    ///
    const std::vector<InteractionPairPtr>& getInteractionPairList() const;

    const std::vector<std::shared_ptr<PbdInteractionPair>>& getPbdPairList() const;

    ///
    /// \brief Returns a map of all interaction pairs per object
    ///
    const std::unordered_map<CollidingObjectPtr, std::vector<InteractionPairPtr>>& getInteractionPairMap() const;

protected:
    std::vector<std::shared_ptr<PbdInteractionPair>> m_interactionPbdPairList;                    //TODO: Refactor -> PBD only

    std::vector<InteractionPairPtr> m_interactionPairList;                                        ///< All interaction pairs in the collision graph
    std::unordered_map<CollidingObjectPtr, std::vector<InteractionPairPtr>> m_interactionPairMap; ///< Map of interaction pairs per colliding object
};
}
