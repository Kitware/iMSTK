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

#include <memory>
#include <unordered_map>
#include <vector>

namespace imstk
{
class SceneObject;
class ObjectInteractionPair;

///
/// \class CollisionGraph
///
/// \brief The CollisionGraph holds a set of interacting pairs of SceneObject's
///
class CollisionGraph
{
public:
    using SceneObjectPtr       = std::shared_ptr<SceneObject>;
    using ObjectInteractionPtr = std::shared_ptr<ObjectInteractionPair>;

    ///
    /// \brief Default constructor
    ///
    CollisionGraph() = default;

    ///
    /// \brief Default destructor
    ///
    ~CollisionGraph() = default;

public:
    ///
    /// \brief Add an interaction pair to the graph
    ///
    void addInteraction(ObjectInteractionPtr pair);

    ///
    /// \brief Remove interaction pair in collision graph
    ///
    bool removeInteractionPair(SceneObjectPtr A, SceneObjectPtr B);
    bool removeInteractionPair(ObjectInteractionPtr intPair);

    ///
    /// \brief Returns the interaction pair if it exists
    ///
    ObjectInteractionPtr getInteractionPair(SceneObjectPtr A, SceneObjectPtr B);

    ///
    /// \brief Returns all interaction pairs
    ///
    const std::vector<ObjectInteractionPtr>& getInteractionPairs() const { return m_interactionPairs; }

    ///
    /// \brief Returns a map of all interaction pairs per object
    ///
    const std::unordered_map<SceneObjectPtr, std::vector<ObjectInteractionPtr>>& getInteractionPairMap() const;

protected:
    std::vector<ObjectInteractionPtr> m_interactionPairs;                                       ///< All interaction pairs in the collision graph
    std::unordered_map<SceneObjectPtr, std::vector<ObjectInteractionPtr>> m_interactionPairMap; ///< Map of interaction pairs per colliding object
};
}
