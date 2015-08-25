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

// STD includes
#include <numeric> // for iota

// SimMedTK includes
#include "Rendering/OpenGLViewer.h"
#include "Collision/SurfaceTreeIterator.h"
#include "Event/KeyboardEvent.h"
#include "Core/Factory.h"
#include "Collision/MeshCollisionModel.h"

/// \brief initialize the surface tree structure
template <typename CellType>
void SurfaceTree<CellType>::initStructure()
{
    if(!this->model->getMesh() || this->model->getMesh()->getNumberOfVertices() == 0)
    {
        std::cerr << "Error: Empty or unvalid mesh." << std::endl;
        return;
    }
    core::Vec3d center;
    std::vector<int> triangles(this->model->getMesh()->getTriangles().size());
    std::iota(std::begin(triangles),std::end(triangles),0);

    root = std::make_shared<CellType>();

    auto &boundingBox = model->getBoundingBox();
    center = boundingBox.center();
    root->setCenter(center);
    root->setLength(boundingBox.sizes().maxCoeff());
    root->setIsEmpty(false);
    root->setAabb(model->getBoundingBox());

    treeAllLevels[0] = *root.get();
    this->createTree(root, triangles, 0);
    initialTreeAllLevels = treeAllLevels;
}

/// \brief destructor
template<typename CellType>
SurfaceTree<CellType>::~SurfaceTree()
{
}

/// \brief
template<typename CellType>
SurfaceTree<CellType>::SurfaceTree(std::shared_ptr<MeshCollisionModel> surfaceModel, int maxLevels)
{
    this->model = surfaceModel;
    totalCells = 0;

    //set the total levels
    currentLevel = maxLevel = maxLevels;

    //compute the total cells
    for (int i = 0; i < maxLevel; ++i)
    {
        totalCells += std::pow(int(CellType::numberOfSubdivisions), i);
    }

    treeAllLevels.resize(totalCells);
    initialTreeAllLevels.resize(totalCells);
    levelStartIndex.resize(maxLevel);

    //compute the levels start and end
    levelStartIndex[0][0] = 0;

    for (int i = 1; i < maxLevel; ++i)
    {
        levelStartIndex[i][0] = levelStartIndex[i-1][0] + std::pow(int(CellType::numberOfSubdivisions), i-1);
        levelStartIndex[i-1][1] = levelStartIndex[i][0];
    }

    levelStartIndex[maxLevel-1][1] = totalCells;

    minTreeRenderLevel = 0;
    renderSurface = false;
    enableShiftPos = false;
    shiftScale = 1.0;
    enableTrianglePos = false;
    renderOnlySurface = false;

    this->model->computeBoundingBoxes();
    this->setRenderDelegate(
      Factory<RenderDelegate>::createConcreteClass(
        "SurfaceTreeRenderDelegate"));
}

/// \brief handle key press events
template<typename CellType>
void SurfaceTree<CellType>::handleEvent(std::shared_ptr<core::Event> event)
{
    if(!this->isListening())
    {
        return;
    }
    auto keyBoardEvent = std::static_pointer_cast<event::KeyboardEvent>(event);
    if(keyBoardEvent != nullptr)
    {
        event::Key keyPressed = keyBoardEvent->getKeyPressed();
        switch(keyPressed)
        {
            case event::Key::Add:
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

            case event::Key::Subtract:
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

            case event::Key::R:
            {
                this->renderSurface = !this->renderSurface;
            }

            case event::Key::P:
            {
                this->enableShiftPos = !this->enableShiftPos;
            }

            case event::Key::K:
            {
                this->renderOnlySurface = !this->renderOnlySurface;
            }

            case event::Key::T:
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
bool SurfaceTree<CellType>::createTree(std::shared_ptr<CellType> Node,
                                         const std::vector<int> &triangles,
                                         int siblingIndex)
{
    std::array<CellType, CellType::numberOfSubdivisions> subDividedNodes;
    std::array<std::vector<int>, CellType::numberOfSubdivisions> triangleMatrix;

    auto const &vertices = this->model->getVertices();
    auto const &meshTriangles = this->model->getTriangles();
    int level = Node->getLevel();

    if(level >= maxLevel)
    {
        return false;
    }

    if (level == maxLevel-1)
    {
        Node->setIsLeaf(true);
        double totalDistance = 0.0;

        for(auto &t : triangles)
        {
            Node->addTriangleData(this->model->getAabb(t),t);
            Node->addVertexIndex(meshTriangles[t][0]);
            Node->addVertexIndex(meshTriangles[t][1]);
            Node->addVertexIndex(meshTriangles[t][2]);
        }
        Node->update();

        for(const auto &i : Node->getVerticesIndices())
        {
            totalDistance += (Node->getCenter() - vertices[i]).norm();
        }

        float weightSum = 0;
        float weight;
        float totalDistance2 = totalDistance * totalDistance;

        for(const auto &i : Node->getVerticesIndices())
        {
            // TODO: make sure this is what is meant: 1-d^2/D^2 and not (1-d^2)/D^2
            weight = 1-(Node->getCenter()-vertices[i]).squaredNorm() / totalDistance2;
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
                        vertices[meshTriangles[triangles[i]][0]],
                        vertices[meshTriangles[triangles[i]][1]],
                        vertices[meshTriangles[triangles[i]][2]]))
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
                for(auto &t : triangleMatrix[j])
                {
                    childNode->addTriangleData(this->model->getAabb(t),t);
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
CollisionModelIterator<CellType> SurfaceTree<CellType>::getLevelIterator(int level)
{
    SurfaceTreeIterator<CellType> iter(this);
    iter.startIndex = iter.currentIndex = this->levelStartIndex[level][0];
    iter.endIndex = this->levelStartIndex[level][1];
    iter.currentLevel = level;
    return iter;
}

/// \brief !!
template <typename CellType>
CollisionModelIterator<CellType>  SurfaceTree<CellType>::getLevelIterator()
{
    SurfaceTreeIterator<CellType> iter(this);
    iter.startIndex = iter.currentIndex = this->levelStartIndex[currentLevel][0];
    iter.endIndex = this->levelStartIndex[currentLevel][1];
    iter.currentLevel = currentLevel;
    return iter;
}

/// \brief update the surface tree
template <typename CellType>
void SurfaceTree<CellType>::updateStructure()
{
    CellType *current;
    auto const &vertices = this->model->getVertices();
    auto const &origVertices = this->model->getMesh()->getOrigVertices();
    for (int i = levelStartIndex[maxLevel-1][0]; i < levelStartIndex[maxLevel-1][1]; ++i)
    {
        current = &treeAllLevels[i];
        core::Vec3d tempCenter(0, 0, 0);
        int counter = 0;

        if (!current->isEmpty())
        {
            for(auto &i : current->getVerticesIndices())
            {
                tempCenter = tempCenter + (vertices[i]-origVertices[i]) * current->getWeight(counter);
                counter++;
            }

            current->getCube().center = tempCenter;
        }
    }
}

/// \brief !!
template <typename CellType>
void SurfaceTree<CellType>::translateRot()
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

template <typename CellType>
void SurfaceTree<CellType>::getIntersectingNodes(const std::shared_ptr<CellType> left,
                                                          const std::shared_ptr<CellType> right,
                                                          std::vector<std::pair<std::shared_ptr<CellType>,std::shared_ptr<CellType>>> &result )
{
    if(left->getAabb().intersection(right->getAabb()).isEmpty())
    {
        return;
    }

    if(left->getIsLeaf() && right->getIsLeaf())
    {
        result.emplace_back(left,right);
    }
    else if(left->getIsLeaf())
    {
        for(const auto &child : right->getChildNodes())
        {
            if(!child) continue;
            this->getIntersectingNodes(left,child,result);
        }
    }
    else if(right->getIsLeaf())
    {
        for(const auto &child : left->getChildNodes())
        {
            if(!child) continue;
            this->getIntersectingNodes(child,right,result);
        }
    }
    else
    {
        for(const auto &rightChild : right->getChildNodes())
        {
            if(!rightChild) continue;
            for(const auto &leftChild : left->getChildNodes())
            {
                if(!leftChild) continue;
                this->getIntersectingNodes(leftChild,rightChild,result);
            }
        }
    }

}



#endif


