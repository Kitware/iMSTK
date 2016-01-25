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

#include "CollisionContext/CollisionContext.h"

#include "Solvers/SystemOfEquations.h"

//---------------------------------------------------------------------------
void CollisionContext::addInteraction(
    std::shared_ptr< SystemOfEquations > sceneObjectA,
    std::shared_ptr< SystemOfEquations > sceneObjectB,
    std::shared_ptr< CollisionDetection > collisionDetection,
    std::shared_ptr< ContactHandling > contactHandlingA,
    std::shared_ptr< ContactHandling > contactHandlingB,
    std::shared_ptr< CollisionManager > contactType, bool active)
{
    if (!sceneObjectA || !sceneObjectB ||
        !collisionDetection || !contactHandlingA ||
        !contactHandlingB || !contactType)
    {
        // TODO: log this
        return;
    }

    if(sceneObjectA != sceneObjectB)
    {
        this->totalNumberOfSceneModels++;
    }

    auto scenePair = std::make_tuple(sceneObjectA, sceneObjectB);
    auto scenePairData = std::make_tuple(collisionDetection,
        contactHandlingA,
        contactHandlingB,
        contactType,
        active);

    this->interactionMap.emplace(scenePair, scenePairData);
}

//---------------------------------------------------------------------------
void CollisionContext::addInteraction(
    std::shared_ptr< SystemOfEquations > sceneObjectA,
    std::shared_ptr< SystemOfEquations > sceneObjectB,
    std::shared_ptr< CollisionDetection > collisionDetection)
{
    if (!sceneObjectA || !sceneObjectB || !collisionDetection)
    {
        // TODO: log this
        return;
    }

    if(sceneObjectA != sceneObjectB)
    {
        this->totalNumberOfSceneModels++;
    }

    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    std::get<Detection>(this->interactionMap[pair]) = collisionDetection;
    std::get<Enabled>(this->interactionMap[pair]) = true;
}

//---------------------------------------------------------------------------
void CollisionContext::addInteraction(
    std::shared_ptr< SystemOfEquations > sceneObjectA,
    std::shared_ptr< SystemOfEquations > sceneObjectB,
    std::shared_ptr< ContactHandling > contactHandler)
{
    if (!sceneObjectA || !sceneObjectB)
    {
        // TODO: log this
        return;
    }

    if(sceneObjectA != sceneObjectB)
    {
        this->totalNumberOfSceneModels++;
    }

    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    std::get<HandlingA>(this->interactionMap[pair]) = contactHandler;
    std::get<HandlingB>(this->interactionMap[pair]) = contactHandler;
    std::get<Enabled>(this->interactionMap[pair]) = true;
}

//---------------------------------------------------------------------------
void CollisionContext::addInteraction(
    std::shared_ptr< SystemOfEquations > sceneObjectA,
    std::shared_ptr< SystemOfEquations > sceneObjectB)
{
    if (!sceneObjectA || !sceneObjectB)
    {
        // TODO: log this
        return;
    }

    if(sceneObjectA != sceneObjectB)
    {
        this->totalNumberOfSceneModels++;
    }

    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    this->interactionMap[pair] = std::make_tuple(nullptr, nullptr, nullptr, nullptr, false);
}

//---------------------------------------------------------------------------
void CollisionContext::disableInteraction(std::shared_ptr< SystemOfEquations > sceneObject1,
                                          std::shared_ptr< SystemOfEquations > sceneObject2)
{
    auto it = this->interactionMap.find(std::make_tuple(sceneObject1, sceneObject2));

    if (it == std::end(this->interactionMap))
    {
        std::cout
            << "Warning: The interaction to be disabled was not found!"
            << std::endl;
    }
    else
    {
        std::get<Enabled>(it->second) = false;
    }
}

//---------------------------------------------------------------------------
void CollisionContext::removeInteraction(
    std::shared_ptr< SystemOfEquations > sceneObjectA,
    std::shared_ptr< SystemOfEquations > sceneObjectB)
{
    if (!sceneObjectA || !sceneObjectB)
    {
        // TODO: log this
        return;
    }

    auto it = this->interactionMap.find(std::make_tuple(sceneObjectA, sceneObjectB));

    if (it == std::end(this->interactionMap))
    {
        // TODO: Log this
        return;
    }

    this->interactionMap.erase(it);
    this->totalNumberOfSceneModels--;
}

//---------------------------------------------------------------------------
void CollisionContext::
setCollisionDetection(std::shared_ptr< SystemOfEquations > sceneObjectA,
                      std::shared_ptr< SystemOfEquations > sceneObjectB,
                      std::shared_ptr< CollisionDetection > collisionDetection)
{
    if (!sceneObjectA || !sceneObjectB || !collisionDetection)
    {
        // TODO: log this
        return;
    }
    this->addInteraction(sceneObjectA, sceneObjectB, collisionDetection);
}

//---------------------------------------------------------------------------
void CollisionContext::
setContactHandling(std::shared_ptr< SystemOfEquations > sceneObjectA,
                   std::shared_ptr< SystemOfEquations > sceneObjectB,
                   std::shared_ptr< ContactHandling > contactHandler)
{
    if (!sceneObjectA || !sceneObjectB || !contactHandler)
    {
        // TODO: log this
        return;
    }
    this->addInteraction(sceneObjectA, sceneObjectB, contactHandler);
}

//---------------------------------------------------------------------------
bool CollisionContext::exist(std::shared_ptr< SystemOfEquations > sceneObject1,
                             std::shared_ptr< SystemOfEquations > sceneObject2)
{
    auto it = this->interactionMap.find(std::make_tuple(sceneObject1, sceneObject2));
    return it != std::end(this->interactionMap);
}

//---------------------------------------------------------------------------
void CollisionContext::createAssemblerAdjacencyMatrix()
{
    // Extract all potential interactive scene objects and create an index map
    // in order to create the adjacency matrix
    int index = 0;
    this->objectIndexMap.clear();

    for (auto & interaction : this->interactionMap)
    {
        auto objectA = std::get<ObjectA>(interaction.first);
        auto objectB = std::get<ObjectB>(interaction.first);

        if (std::end(this->objectIndexMap) == this->objectIndexMap.find(objectA))
        {
            this->objectIndexMap.emplace(objectA, index++);
        }

        if (std::end(this->objectIndexMap) == this->objectIndexMap.find(objectB))
        {
            this->objectIndexMap.emplace(objectB, index++);
        }
    }

    // Populate the triplets for the adjacency matrix
    auto numSystemOfEquationss = this->objectIndexMap.size();

    // set the size of the adjacency matrix
    this->interactionMatrix.resize(numSystemOfEquationss);
    for (size_t i = 0; i < numSystemOfEquationss; ++i)
    {
        this->interactionMatrix[i].resize(numSystemOfEquationss);
    }

    std::vector<Eigen::Triplet<int>> triplets;

    for (auto & interaction : this->interactionMap)
    {
        if (!std::get<Enabled>(interaction.second))
        {
            continue;
        }

        auto objectA = std::get<0>(interaction.first);
        auto objectB = std::get<1>(interaction.first);

        auto i = this->objectIndexMap.find(objectA);
        auto j = this->objectIndexMap.find(objectB);

        if (i == std::end(this->objectIndexMap) || j == std::end(this->objectIndexMap))
        {
            std::cout << "Warning: scene object does not have an index" << std::endl;
            continue;
        }
        else
        {
            this->interactionMatrix[i->second][j->second] = 1;
        }
    }

    for (auto & st : this->modelPairs)//
    {
        auto i = this->objectIndexMap.find(st.first);
        auto j = this->objectIndexMap.find(st.second);

        if (i == std::end(this->objectIndexMap) || j == std::end(this->objectIndexMap))
        {
            std::cout << "Warning: scene object does not have an index" << std::endl;
            continue;
        }
        else
        {
            this->interactionMatrix[i->second][j->second] = 1;
        }

    }
}

//---------------------------------------------------------------------------
void CollisionContext::solveSimultaneously(std::shared_ptr< SystemOfEquations > sceneObjectA,
                                           std::shared_ptr< SystemOfEquations > sceneObjectB)
{
    auto i = this->objectIndexMap.find(sceneObjectA);
    auto j = this->objectIndexMap.find(sceneObjectB);

    if (i == std::end(this->objectIndexMap) || j == std::end(this->objectIndexMap))
    {
        std::cout << "Warning: scene object does not have an index" << std::endl;
    }
    else
    {
        this->modelPairs.emplace_back(sceneObjectA, sceneObjectB);
    }
}

//---------------------------------------------------------------------------
bool CollisionContext::configure()
{
    this->createAssemblerAdjacencyMatrix();
    return true;
}

//---------------------------------------------------------------------------
std::size_t CollisionContext::getNumberOfInteractions() const
{
    return this->interactionMap.size();
}

//---------------------------------------------------------------------------
const CollisionContext::InteractionMapType &CollisionContext::getInteractions() const
{
    return this->interactionMap;
}

//---------------------------------------------------------------------------
std::vector<std::shared_ptr<ContactHandling>> CollisionContext::getContactHandlers()
{
    std::vector<std::shared_ptr<ContactHandling>> handlerList;

    for (auto & interaction : this->interactionMap)
    {
        if (!std::get<Enabled>(interaction.second))
        {
            continue;
        }

        auto hA = std::get<HandlingA>(interaction.second);
        auto hB = std::get<HandlingB>(interaction.second);

        if (hA)
        {
            handlerList.emplace_back(hA);
        }

        if (hB)
        {
            handlerList.emplace_back(hB);
        }
    }

    return handlerList;
}

//---------------------------------------------------------------------------
void CollisionContext::findIslands()
{

}

//---------------------------------------------------------------------------
size_t CollisionContext::getNumOfIslands()
{
    return this->islands.size();
}

//---------------------------------------------------------------------------
std::vector<std::vector<int>>& CollisionContext::getIslands()
{
    return this->islands;
}

//---------------------------------------------------------------------------
void CollisionContext::formIslands()
{
    if(this->totalNumberOfSceneModels == 0)
    {
        // TODO: Log this
        return;
    }

    std::vector<bool> visited(this->totalNumberOfSceneModels,false);

    int nIslands = 0;
    for (int i = 0; i < this->totalNumberOfSceneModels; i++)
    {
        if (!visited[i])
        {
            std::vector<int> memberList;
            memberList.push_back(i);
            visited[i] = true;
            nIslands++;
            this->appendNeighbors(visited, memberList, i);

            this->islands.push_back(memberList);
        }
    }
}

//---------------------------------------------------------------------------
void CollisionContext::appendNeighbors(std::vector<bool>& visited,
                                       std::vector<int>& memberList,
                                       int row)
{
    // NOTE: only check the upper triangular matrix for now
    for (size_t col = row, end = interactionMatrix.size(); col < end; ++col)
    {
        const auto &entry = interactionMatrix[row][col];
        if (entry != 0 &&
            std::find(std::begin(memberList), std::end(memberList), entry) != std::end(memberList))
        {
            memberList.push_back(col);
            if (!visited[col])
            {
                this->appendNeighbors(visited, memberList, col);
                visited[col] = true;
            }
        }
    }
}
