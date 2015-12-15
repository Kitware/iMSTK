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

#include "CollisionContext/CollisionContext.h"

InteractionDataElement::InteractionDataElement(
    int so1,
    int so2,
    core::CollisionDetectionType cd,
    core::ContactHandlingType ch1,
    core::ContactHandlingType ch2,
    core::CollisionPairType cp)
{
    sceneObj1Id = so1;
    sceneObj2Id = so2;
    collDetectionType = cd;
    contHandlingType1 = ch1;
    contHandlingType2 = ch2;
    collPairType = cp;
}

//---------------------------------------------------------------------------
bool CollisionContext::addInteraction(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB,
    std::shared_ptr< CollisionDetection > collisionDetection,
    std::shared_ptr< ContactHandling > contactHandlingA,
    std::shared_ptr< ContactHandling > contactHandlingB,
    std::shared_ptr< CollisionDataBase > contactType, bool active)
{
    if (!sceneObjectA ||
        !sceneObjectB ||
        !collisionDetection ||
        !contactHandlingA ||
        !contactHandlingB ||
        !contactType)
    {
        return false;
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
bool CollisionContext::addInteraction(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB,
    std::shared_ptr< CollisionDetection > collisionDetection)
{
    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    std::get<Detection>(this->interactionMap[pair]) = collisionDetection;
    std::get<Enabled>(this->interactionMap[pair]) = true;
}

//---------------------------------------------------------------------------
bool CollisionContext::addInteraction(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB,
    std::shared_ptr< ContactHandling > contactHandler)
{
    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    std::get<HandlingA>(this->interactionMap[pair]) = contactHandler;
    std::get<HandlingB>(this->interactionMap[pair]) = contactHandler;
    std::get<Enabled>(this->interactionMap[pair]) = true;
}

//---------------------------------------------------------------------------
bool CollisionContext::addInteraction(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB)
{
    auto pair = std::make_tuple(sceneObjectA, sceneObjectB);
    this->interactionMap[pair] = std::make_tuple(nullptr, nullptr, nullptr, nullptr, false);
}

//---------------------------------------------------------------------------
void CollisionContext::disableInteraction(
    std::shared_ptr< SceneObject > sceneObject1,
    std::shared_ptr< SceneObject > sceneObject2)
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
    std::shared_ptr< SceneObject > sceneObject1,
    std::shared_ptr< SceneObject > sceneObject2)
{
    auto it = this->interactionMap.find(std::make_tuple(sceneObject1, sceneObject2));

    if (it == std::end(this->interactionMap))
    {
        std::cout
            << "Warning: The interaction to be disabled is not found!"
            << std::endl;
    }

    else
    {
        this->interactionMap.erase(it);
    }
}

//---------------------------------------------------------------------------
bool CollisionContext::setCollisionDetection(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB,
    std::shared_ptr< CollisionDetection > collisionDetection)
{
    this->addInteraction(sceneObjectA, sceneObjectB, collisionDetection);
}

//---------------------------------------------------------------------------
bool CollisionContext::setContactHandling(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB,
    std::shared_ptr< ContactHandling > contactHandler)
{
    this->addInteraction(sceneObjectA, sceneObjectB, contactHandler);
}

//---------------------------------------------------------------------------
bool CollisionContext::exist(
    std::shared_ptr< SceneObject > sceneObject1,
    std::shared_ptr< SceneObject > sceneObject2)
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
    auto numSceneObjects = this->objectIndexMap.size();

    // set the size of the adjacency matrix
    this->interactionMatrix.resize(numSceneObjects);
    for (int i = 0; i < numSceneObjects; ++i)
    {
        this->interactionMatrix[i].resize(numSceneObjects);
    }

    std::vector<Eigen::Triplet<int>> triplets;

    // Populate the assembly adjacency matrix
    int i = 0, j = 0;

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
            this->interactionMatrix[i.second][j.second] = 1;
        }
    }

    for (auto & st : this->solveTogether)//
    {
        auto objA = this->objectIndexMap.find(st.first);
        auto objB = this->objectIndexMap.find(st.second);

        if (objA == std::end(this->objectIndexMap) || objB == std::end(this->objectIndexMap))
        {
            std::cout << "Warning: scene object does not have an index" << std::endl;
            continue;
        }
        else
        {
            this->interactionMatrix[objA.second][objB.second] = 1;
        }

    }
}
//---------------------------------------------------------------------------
void CollisionContext::solveTogether(
    std::shared_ptr< SceneObject > sceneObjectA,
    std::shared_ptr< SceneObject > sceneObjectB)
{
    auto i = this->objectIndexMap.find(sceneObjectA);
    auto j = this->objectIndexMap.find(sceneObjectB);

    if (i == std::end(this->objectIndexMap) || j == std::end(this->objectIndexMap))
    {
        std::cout << "Warning: scene object does not have an index" << std::endl;
    }
    else
    {
        auto objPair = std::make_pair(sceneObjectA, sceneObjectB);
        this->solveTogether.push_back(objPair);
    }
}

//---------------------------------------------------------------------------
bool CollisionContext::configure()
{
    this->createAssemblerAdjacencyMatrix();
    return true;
}

//---------------------------------------------------------------------------
std::size_t CollisionContext::getNumberOfInterations() const
{
    return this->interactionMap.size();
}

//---------------------------------------------------------------------------
CollisionContext::InteractionMapType &CollisionContext::getInteractions() const
{
    return this->interactionMap;
}

//---------------------------------------------------------------------------
std::vector< std::shared_ptr< ContactHandling > > CollisionContext::getContactHandlers()
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

    return std::move(handlerList);
}

//---------------------------------------------------------------------------
void CollisionContext::findIslands()
{

}

int CollisionContext::getNumOfIslands()
{
    return this->islands->size();
}

std::vector<std::vector<int>>& CollisionContext::getIslands()
{
    return this->islands;
}

std::shared_ptr<SceneObject> CollisionContext::getObjectWithIndex(const int objIndex)
{
    for (int i = 0; i < this->objectIndexMap.size(); ++i)
    {
        if (this->objectIndexMap.at(i).second == objIndex)
        {
            return this->objectIndexMap.at(i).second;
        }
    }
}

//---------------------------------------------------------------------------
template<typename T>
int CollisionContext::numberOfInteractions() const
{
    int count = 0;
    auto counter = [&](const InteractionPairDataType &data)
    {
        if (std::dynamic_pointer_cast<T>(std::get<HandlingA>(data)) ||
            std::dynamic_pointer_cast<T>(std::get<HandlingB>(data)))
        {
            ++count;
        }
    };
    std::for_each(this->interactionMap.begin(), interactionMap.end(), counter);
    return count;
}

//---------------------------------------------------------------------------
// \todo test
void CollisionContext::formIslands()
{
    int nSceneObj, nIslands;
    std::vector<bool> visited;
    visited.resize(nSceneObj);

    nIslands = 0;
    for (int i = 0; i < nSceneObj; i++)
    {
        if (!visited[i])
        {
            nIslands++;
            std::vector<int> memberList;
            appendNeighbors(visited, memberList, i);

            visited[i] = 1;

            this->islands.push_back(memberList);
        }
    }
}

//---------------------------------------------------------------------------
// \todo test
void CollisionContext::appendNeighbors(
    std::vector<bool>& visited, std::vector<int>& memberList, int row)
{
    for (int i = row; i < interactionMatrix.size(); ++i)// only check the upper triangular matrix for now
    {
        if (interactionMatrix[row][i] != 0
            && std::find(memberList.begin(), memberList.end(), interactionMatrix[row][i]) != memberList.end())
        {
            memberList.push_back(interactionMatrix[row][i]);
            if (!visited[interactionMatrix[row][i]])
            {
                appendNeighbors(visited, memberList, interactionMatrix[row][i]);
            }
        }
    }
}