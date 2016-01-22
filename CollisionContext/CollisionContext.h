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

#ifndef COLLISION_CONTEXT_COLLISION_CONTEXT_H
#define COLLISION_CONTEXT_COLLISION_CONTEXT_H

// iMSTK includes
#include "Core/CoreClass.h"
#include "Core/Matrix.h"
#include "Core/Config.h"
#include "SceneModels/SceneObject.h"

// collision detection includes
#include "Core/CollisionDetection.h"
#include "Core/CollisionManager.h"
#include "Collision/PlaneToMeshCollision.h"
#include "Collision/SpatialHashCollision.h"

// contact handling includes
#include "Core/ContactHandling.h"
#include "ContactHandling/PenaltyContactFemToStatic.h"

///
/// \brief This class manages all the information related to
/// interactions of scene objects within the scene
///
class CollisionContext
{
public:
    // Use these to access interaction pair data from the map with the std::get method
    enum { Detection = 0, HandlingA, HandlingB, Data, Enabled };

    // Use these to access pair objects
    enum { ObjectA = 0, ObjectB };

    /// This type hold a pair of potential scene objects that need to
    /// be queried for interaction
    using InteractionPairType = std::tuple<std::shared_ptr<SceneObject>,
                                           std::shared_ptr<SceneObject>> ;

    /// This type holds the algorithms and data types for the interaction pair
    using InteractionPairDataType = std::tuple<std::shared_ptr<CollisionDetection>,
                                               std::shared_ptr<ContactHandling>,
                                               std::shared_ptr<ContactHandling>,
                                               std::shared_ptr<CollisionManager>,
                                               bool> ;

    /// Stores interaction types, their algorithms and data
    struct HashType
    {
        size_t operator()(const InteractionPairType &pair) const
        {
            size_t h1 = std::hash<std::shared_ptr<SceneObject>>()(std::get<ObjectA>(pair));
            size_t h2 = std::hash<std::shared_ptr<SceneObject>>()(std::get<ObjectB>(pair));
            return h1 ^ (h2 << 1);
        };
    };
    struct ComparisonType
    {
        bool operator()(const InteractionPairType &pairA,
                        const InteractionPairType &pairB) const
        {
            auto pairC = std::make_tuple(std::get<ObjectB>(pairA), std::get<ObjectA>(pairA));
            return pairA == pairB || pairC == pairB;
        };
    };

    using InteractionMapType = std::unordered_map<InteractionPairType,
                                                  InteractionPairDataType,
                                                  HashType,
                                                  ComparisonType>;

public:
    ///
    /// \brief Constructor/Destructor
    ///
    CollisionContext() : totalNumberOfSceneModels(0) {}
    ~CollisionContext() = default;

    ///
    /// \brief Adds two given scene objects will interact
    /// in the scene to the collision context. Assigns collision
    /// detection method. Assigns the different contact handling methods
    /// different directions
    ///
    /// \note Scene objects passed as arguments should be registered
    ///
    void addInteraction(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB,
        std::shared_ptr<CollisionDetection> collisionDetection,
        std::shared_ptr<ContactHandling> contactHandlingA,
        std::shared_ptr<ContactHandling> contactHandlingB,
        std::shared_ptr<CollisionManager> contactType,
        bool active = true);

    ///
    /// \brief Adds two given scene objects will interact
    /// in the scene to the collision context. Assigns collision
    /// detection method. This does not assign contact handler.
    ///
    void addInteraction(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB,
        std::shared_ptr<CollisionDetection> collisionDetection);

    ///
    /// \brief Adds two given scene objects will interact
    ///  in the scene to the collision context. Assigns contact handling method.
    ///
    void addInteraction(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB,
        std::shared_ptr<ContactHandling> contactHandler);

    ///
    /// \brief Adds two given scene objects that will interact in the scene to the
    ///  collision context
    ///
    void addInteraction(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB);

    ///
    /// \brief Disables interaction between two given scene objects
    ///
    void disableInteraction(std::shared_ptr<SceneObject> sceneObject1,
        std::shared_ptr<SceneObject> sceneObject2);

    ///
    /// \brief Removes interaction from the context between two given scene objects
    ///
    void removeInteraction(std::shared_ptr<SceneObject> sceneObject1,
        std::shared_ptr<SceneObject> sceneObject2);

    ///
    /// \brief Assign a collision detection method between two scene given scene objects
    ///
    void setCollisionDetection(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB,
        std::shared_ptr<CollisionDetection> collisionDetection);

    ///
    /// \brief Assign a contact handler method between two scene given scene objects
    ///
    void setContactHandling(std::shared_ptr<SceneObject> sceneObjectA,
        std::shared_ptr<SceneObject> sceneObjectB,
        std::shared_ptr<ContactHandling> contactHandler);

    ///
    /// \brief check if the interaction between two scene objects
    /// already exists
    ///
    bool exist(
        std::shared_ptr<SceneObject> sceneObject1,
        std::shared_ptr<SceneObject> sceneObject2);

    ///
    /// \brief Populate the assembler adjacency matrix based on input
    /// interactions
    ///
    void createAssemblerAdjacencyMatrix();

    ///
    /// \brief Count the number of interaction of type T.
    ///
    template<typename T>
    size_t numberOfInteractions() const
    {
        size_t count = 0;
        auto counter = [&](const InteractionPairDataType &data)
        {
            if (std::dynamic_pointer_cast<T>(std::get<HandlingA>(data)) ||
                std::dynamic_pointer_cast<T>(std::get<HandlingB>(data)))
            {
                ++count;
            }
        };
        std::for_each(this->interactionMap.begin(), this->interactionMap.end(), counter);
        return count;
    }

    ///
    /// \brief Forms the islands based on the adjacency matrix
    ///
    void formIslands();

    ///
    /// \brief Utility function to add neighbors to a particular island
    ///  This is called recursively
    ///
    void appendNeighbors(
        std::vector<bool>& visited, std::vector<int>& memberList, int row);


    ///
    /// \brief use the information to first derive adjacency matrices for
    /// collision detection, collision response, assembler and solver
    ///
    bool configure();

    ///
    /// \brief returns the number of interactions defined in the
    /// collision context. Maximum is nC2 where n is the total
    /// number of objects in the scene
    ///
    size_t getNumberOfInteractions() const;

    ///
    /// \brief Returns the list of interactions defined
    ///
    const InteractionMapType& getInteractions() const;

    ///
    /// \brief Returns the list of contact handlers
    ///
    std::vector<std::shared_ptr<ContactHandling>> getContactHandlers();

    ///
    /// \brief Find the islands in the adjacency matrix
    ///
    void findIslands();

    ///
    /// \brief Find the islands in the adjacency matrix
    ///
    size_t getNumOfIslands();

    ///
    /// \brief Return the island graph.
    ///
    /// \return Reference to graph.
    ///
    std::vector<std::vector<int>>& getIslands();

    ///
    /// \brief Add pair of scene models to the list to they can be resolved simultaneously.
    ///
    /// \param sceneObjectA
    /// \param sceneObjectB
    ///
    void solveSimultaneously(std::shared_ptr<SceneObject> sceneObjectA,
                             std::shared_ptr<SceneObject> sceneObjectB);

    ///
    /// \brief Get the scene model associated with ith index if exists.
    ///
    /// \param index
    ///
    std::shared_ptr<SceneObject> getSceneModel(int index)
    {
        auto fn = [=](const std::unordered_map<std::shared_ptr<SceneObject>,int>::value_type& vt)
        {
            return vt.second == index;
        };
        auto it = std::find_if(std::begin(this->objectIndexMap), std::end(this->objectIndexMap),fn);
        if(it != std::end(this->objectIndexMap))
        {
            return it->first;
        }
        return nullptr;
    }

private:
    std::vector<std::vector<int>> interactionMatrix; ///> Adjacency matrix for the
                                                     ///> assembly graph (undirected)

    InteractionMapType interactionMap;
    std::unordered_map<std::shared_ptr<SceneObject>, int> objectIndexMap;

    std::list<std::pair<std::shared_ptr<SceneObject>,
                        std::shared_ptr<SceneObject>>> modelPairs;

    std::vector<std::vector<int>> islands;
    int totalNumberOfSceneModels;
};

#endif // SM_COLLISION_CONTEXT
