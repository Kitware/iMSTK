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

#ifndef SM_SMSURFACETREE_HPP
#define SM_SMSURFACETREE_HPP

// STL includes
#include <string>

// SimMedTK includes
#include "smRendering/smViewer.h"
#include "smCollision/smSurfaceTreeIterator.h"
#include "smEvent/smKeyboardEvent.h"
#include "smCore/smFactory.h"

/// \brief initialize the surface tree structure
template <typename CellType>
void smSurfaceTree<CellType>::initStructure()
{
    smVec3d center;
    double edge;
    std::vector<int> triangles;

    for (int i = 0; i < mesh->nbrTriangles; ++i)
    {
        triangles.push_back(i);
    }
    root = std::make_shared<CellType>();

    center = mesh->aabb.center();
    edge = SIMMEDTK_MAX(SIMMEDTK_MAX(mesh->aabb.halfSizeX(), mesh->aabb.halfSizeY()),
                        mesh->aabb.halfSizeZ());
    root->setCenter(center);
    root->setLength(2 * edge);
    root->setIsEmpty(false);
    root->setAabb(mesh->aabb);

    treeAllLevels[0] = *root.get();
    this->createTree(root, triangles, 0);
	initialTreeAllLevels = treeAllLevels;
}

/// \brief destructor
template<typename CellType>
smSurfaceTree<CellType>::~smSurfaceTree()
{
}

/// \brief
template<typename CellType>
smSurfaceTree<CellType>::smSurfaceTree(std::shared_ptr<smSurfaceMesh> surfaceMesh, int maxLevels)
{
    mesh = surfaceMesh;
    totalCells = 0;

    //set the total levels
    currentLevel = maxLevel = maxLevels;

    //compute the total cells
    for (int i = 0; i < maxLevel; ++i)
    {
        totalCells += smMath::pow(CellType::numberOfSubdivisions, i);
    }

    treeAllLevels.resize(totalCells);
    initialTreeAllLevels.resize(totalCells);
    levelStartIndex.resize(maxLevel);

    //compute the levels start and end
    levelStartIndex[0][0] = 0;

    for (int i = 1; i < maxLevel; ++i)
    {
        levelStartIndex[i][0] = levelStartIndex[i-1][0] + smMath::pow(CellType::numberOfSubdivisions, i-1);
        levelStartIndex[i-1][1] = levelStartIndex[i][0];
    }

    levelStartIndex[maxLevel-1][1] = totalCells;

    minTreeRenderLevel = 0;
    renderSurface = false;
    enableShiftPos = false;
    shiftScale = 1.0;
    enableTrianglePos = false;
    renderOnlySurface = false;

    mesh->allocateAABBTris();
    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createConcreteClass(
        "smSurfaceTreeRenderDelegate"));
}

/// \brief handle key press events
template<typename CellType>
void smSurfaceTree<CellType>::handleEvent(std::shared_ptr<smtk::Event::smEvent> event)
{
    if(!this->isListening())
    {
        return;
    }
    auto keyBoardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(event);
    if(keyBoardEvent != nullptr)
    {
        smtk::Event::smKey keyPressed = keyBoardEvent->getKeyPressed();
        switch(keyPressed)
        {
            case smtk::Event::smKey::Add:
            {
                minTreeRenderLevel++;

                if(minTreeRenderLevel > maxLevel)
                {
                    minTreeRenderLevel = maxLevel;
                }

                if(minTreeRenderLevel < 0)
                {
                    minTreeRenderLevel = 0;
                }

                currentLevel = minTreeRenderLevel;
            }

            case smtk::Event::smKey::Subtract:
            {
                minTreeRenderLevel--;

                if(minTreeRenderLevel > maxLevel)
                {
                    minTreeRenderLevel = maxLevel;
                }

                if(minTreeRenderLevel < 0)
                {
                    minTreeRenderLevel = 0;
                }

                currentLevel = minTreeRenderLevel;
            }

            case smtk::Event::smKey::R:
            {
                this->renderSurface = !this->renderSurface;
            }

            case smtk::Event::smKey::P:
            {
                this->enableShiftPos = !this->enableShiftPos;
            }

            case smtk::Event::smKey::K:
            {
                this->renderOnlySurface = !this->renderOnlySurface;
            }

            case smtk::Event::smKey::T:
            {
                updateStructure();
            }

            default:
                std::cerr << "Unhandled key." << std::endl;
        }
    }
}

/// \brief create the surface tree
template<typename CellType>
bool smSurfaceTree<CellType>::createTree(std::shared_ptr<CellType> Node,
                                         const std::vector<int> &triangles,
                                         int siblingIndex)
{
    std::array<CellType, CellType::numberOfSubdivisions> subDividedNodes;
    std::array<std::vector<int>, CellType::numberOfSubdivisions> triangleMatrix;

    int level = Node->getLevel();

    if(level >= maxLevel)
    {
        return false;
    }

    if (level == maxLevel-1)
    {
        Node->setIsLeaf(true);
        double totalDistance = 0.0;

        for(auto &triangle : triangles)
        {
            Node->addTriangleData(mesh->triAABBs[triangle],triangle);
            Node->addVertexIndex(mesh->triangles[triangle].vert[0]);
            Node->addVertexIndex(mesh->triangles[triangle].vert[1]);
            Node->addVertexIndex(mesh->triangles[triangle].vert[2]);
        }
        Node->update();

        for(const auto &i : Node->getVerticesIndices())
        {
            totalDistance += (Node->getCenter() - mesh->vertices[i]).norm();
        }

        smFloat weightSum = 0;
        smFloat weight;
        smFloat totalDistance2 = totalDistance * totalDistance;

        for(const auto &i : Node->getVerticesIndices())
        {
            // TODO: make sure this is what is meant: 1-d^2/D^2 and not (1-d^2)/D^2
            weight = 1-(Node->getCenter()-mesh->vertices[i]).squaredNorm() / totalDistance2;
            weightSum += weight;
            Node->addWeight(weight);
        }

        for(auto &w : Node->getWeights())
        {
            w /= weightSum;
        }

        return true;
    }

    Node->subDivide(2, subDividedNodes);

    for (int i = 0; i < CellType::numberOfSubdivisions; ++i)
    {
        //aabb[i].expand(0.2);
        subDividedNodes[i].expand(0.01);
    }

    for (int i = 0; i < triangles.size(); ++i)
    {
        for (int j = 0; j < CellType::numberOfSubdivisions; ++j)
        {
            if (subDividedNodes[j].isCollidedWithTri(
                        mesh->vertices[mesh->triangles[triangles[i]].vert[0]],
                        mesh->vertices[mesh->triangles[triangles[i]].vert[1]],
                        mesh->vertices[mesh->triangles[triangles[i]].vert[2]]))
            {
                triangleMatrix[j].push_back(triangles[i]);
            }
        }
    }

    int parentLevel = level == 0 ? 0 : level-1;
    int offset = CellType::numberOfSubdivisions * (siblingIndex-levelStartIndex[parentLevel][0]);

    for (int j = 0; j < CellType::numberOfSubdivisions; ++j)
    {

        int childIndex = levelStartIndex[level][1] + offset + j;

        if (triangleMatrix[j].size() > 0)
        {
            std::shared_ptr<CellType> childNode = std::make_shared<CellType>();
            childNode->copyShape(subDividedNodes[j]);
            childNode->setLevel(level + 1);
            childNode->setParentNode(Node);
            Node->setChildNode(j,childNode);

            treeAllLevels[childIndex] = *childNode.get();

            // Set triangle and aabb data for the child node
            if(childNode->getLevel() != maxLevel-1)
            {
                for(auto &triangle : triangleMatrix[j])
                {
                    childNode->addTriangleData(this->mesh->triAABBs[triangle],triangle);
                }
                childNode->update();
            }

            createTree(childNode, triangleMatrix[j], levelStartIndex[level][0] + offset + j);
        }

        triangleMatrix[j].clear();
    }

    return true;
}

/// \brief !!
template <typename CellType>
smCollisionModelIterator<CellType> smSurfaceTree<CellType>::getLevelIterator(int level)
{
    smSurfaceTreeIterator<CellType> iter(this);
    iter.startIndex = iter.currentIndex = this->levelStartIndex[level][0];
    iter.endIndex = this->levelStartIndex[level][1];
    iter.currentLevel = level;
    return iter;
}

/// \brief !!
template <typename CellType>
smCollisionModelIterator<CellType>  smSurfaceTree<CellType>::getLevelIterator()
{
    smSurfaceTreeIterator<CellType> iter(this);
    iter.startIndex = iter.currentIndex = this->levelStartIndex[currentLevel][0];
    iter.endIndex = this->levelStartIndex[currentLevel][1];
    iter.currentLevel = currentLevel;
    return iter;
}

/// \brief update the surface tree
template <typename CellType>
void smSurfaceTree<CellType>::updateStructure()
{
    CellType *current;

    for (int i = levelStartIndex[maxLevel-1][0]; i < levelStartIndex[maxLevel-1][1]; ++i)
    {
        current = &treeAllLevels[i];
        smVec3d tempCenter(0, 0, 0);
        int counter = 0;

        if (!current->isEmpty())
        {
            for(auto &i : current->getVerticesIndices())
            {
                tempCenter = tempCenter + (mesh->vertices[i]-mesh->origVerts[i]) * current->getWeight(counter);
                counter++;
            }

            current->getCube().center = tempCenter;
        }
    }
}

/// \brief !!
template <typename CellType>
void smSurfaceTree<CellType>::translateRot()
{
    CellType *current;
    CellType *initial;

    for (int i = levelStartIndex[maxLevel-1][0];
            i < levelStartIndex[maxLevel-1][1]; ++i)
    {

        current = &treeAllLevels[i];
        initial = &initialTreeAllLevels[i];

        if (!current->isEmpty())
        {
            current->getCube().center = this->transRot.block(0,0,3,3) * initial->getCube().center + this->transRot.col(3).head(3);
        }
    }
}



#endif

