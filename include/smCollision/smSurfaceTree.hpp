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

// Eigen includes
#include "Eigen/Dense"

// SimMedTK includes
#include "smRendering/smViewer.h"

/// \brief initialize the surface tree structure
template <typename CellType>
void smSurfaceTree<CellType>::initStructure()
{
    smVec3f center;
    float edge;
    std::vector<int> triangles;

    for (int i = 0; i < mesh->nbrTriangles; i++)
    {
        triangles.push_back(i);
    }

    center = mesh->aabb.center();
    edge = SIMMEDTK_MAX(SIMMEDTK_MAX(mesh->aabb.halfSizeX(), mesh->aabb.halfSizeY()),
                        mesh->aabb.halfSizeZ());
    root.setCenter(center);
    root.setLength(2 * edge);
    root.filled = true;

    treeAllLevels[0] = root;
    this->createTree(treeAllLevels[0], triangles, 0, 0);
	initialTreeAllLevels = treeAllLevels;
}

/// \brief destructor
template<typename CellType>
smSurfaceTree<CellType>::~smSurfaceTree()
{
}

/// \brief
template<typename CellType>
smSurfaceTree<CellType>::smSurfaceTree(smSurfaceMesh *mesh, int maxLevels)
{
    mesh = mesh;
    totalCells = 0;

	//set the total levels
    currentLevel = maxLevel = maxLevels;

    //compute the total cells
    for (int i = 0; i < maxLevel; i++)
    {
        totalCells += smMath::pow(CellType::numberOfSubdivisions, i);
    }

    treeAllLevels.resize(totalCells);
    initialTreeAllLevels.resize(totalCells);
    levelStartIndex.resize(maxLevel);

    //compute the levels start and end
    int previousIndex = totalCells;
    levelStartIndex[0][0] = 0;
    levelStartIndex[0][1] = 1;

    for (int i = 1; i < maxLevel; i++)
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
}

/// \brief Initialize the drawing structures
template<typename CellType>
void smSurfaceTree<CellType>::initDraw(smDrawParam param)
{
    smViewer *viewer;
    viewer = param.rendererObject;
    viewer->addText("octree");
}

/// \brief draw the surface tree
template<typename CellType>
void smSurfaceTree<CellType>::draw(smDrawParam params)
{
    smVec3f center;
    float length;
    glColor3fv(smColor::colorGreen.toGLColor());

    glEnable(GL_LIGHTING);
    glPushAttrib(GL_LIGHTING_BIT);
    glColor3fv(smColor::colorGreen.toGLColor());
    glColor3fv(smColor::colorBlue.toGLColor());

    static int counter = 0;
    if (renderOnlySurface == false)
    {
        for (int i = levelStartIndex[minTreeRenderLevel][0];
                i < levelStartIndex[minTreeRenderLevel][1]; i++)
        {
            center = treeAllLevels[i].getCenter();
            length = treeAllLevels[i].getLength();

            if (treeAllLevels[i].filled)
            {
                glPushMatrix();
                glColor3fv(smColor::colorPink.toGLColor());
                glTranslatef(center[0], center[1], center[2]);
                glutSolidSphere(length, 10, 10);
                glPopMatrix();
                counter++;
            }
        }
    }

    glPopAttrib();
    glEnable(GL_LIGHTING);
    params.rendererObject->updateText("octree", "Total Spheres at Level:" + std::to_string(counter));
}

/// \brief handle key press events
template<typename CellType>
void smSurfaceTree<CellType>::handleEvent(smEvent *event)
{

    smKeyboardEventData *keyBoardData;

    switch (event->eventType.eventTypeCode)
    {

    case SIMMEDTK_EVENTTYPE_KEYBOARD:

        keyBoardData = (smKeyboardEventData*)event->data;

        if (keyBoardData->keyBoardKey == smKey::Add)
        {
            minTreeRenderLevel++;

            if (minTreeRenderLevel > maxLevel)
            {
                minTreeRenderLevel = maxLevel;
            }

            if (minTreeRenderLevel < 0)
            {
                minTreeRenderLevel = 0;
            }

            currentLevel = minTreeRenderLevel;
        }

        if (keyBoardData->keyBoardKey == smKey::Subtract)
        {
            minTreeRenderLevel--;

            if (minTreeRenderLevel > maxLevel)
            {
                minTreeRenderLevel = maxLevel;
            }

            if (minTreeRenderLevel < 0)
            {
                minTreeRenderLevel = 0;
            }

            currentLevel = minTreeRenderLevel;
        }

        if (keyBoardData->keyBoardKey == smKey::R)
        {
            this->renderSurface = !this->renderSurface;
        }

        if (keyBoardData->keyBoardKey == smKey::P)
        {
            this->enableShiftPos = !this->enableShiftPos;
        }

        if (keyBoardData->keyBoardKey == smKey::K)
        {
            this->renderOnlySurface = !this->renderOnlySurface;
        }

        if (keyBoardData->keyBoardKey == smKey::T)
        {
            updateStructure();
        }

        break;
    }
}

/// \brief create the surface tree
template<typename CellType>
bool smSurfaceTree<CellType>::createTree(CellType &Node,
										 const std::vector<int> &triangles,
										 int level,
										 int siblingIndex)
{

	std::vector<CellType> subDividedNodes(CellType::numberOfSubdivisions);
	std::array<std::vector<int>,CellType::numberOfSubdivisions> triangleMatrix;

    if (level >= maxLevel)
    {
        return false;
    }

    Node.level = level;


    if (level == maxLevel-1)
    {

        int nbrTriangles = triangles.size();
        float totalDistance = 0.0;

        for (int i = 0; i < nbrTriangles; i++)
        {

            Node.verticesIndices.insert(mesh->triangles[triangles[i]].vert[0]);
            Node.verticesIndices.insert(mesh->triangles[triangles[i]].vert[1]);
            Node.verticesIndices.insert(mesh->triangles[triangles[i]].vert[2]);
        }

        for (std::set<int>::iterator it = Node.verticesIndices.begin(); it != Node.verticesIndices.end(); it++)
        {
            totalDistance += (Node.getCube().center - mesh->vertices[*it]).norm();
        }

        float weightSum = 0;
        float weight;

        for (std::set<int>::iterator it = Node.verticesIndices.begin(); it != Node.verticesIndices.end(); it++)
        {
            weight = 1-(Node.getCenter().distance(mesh->vertices[*it]) * Node.getCenter().distance(mesh->vertices[*it])) / (totalDistance * totalDistance);
            weight = 1-(Node.getCenter().distance(mesh->vertices[*it]) * Node.getCenter().distance(mesh->vertices[*it])) / (totalDistance * totalDistance);
            weightSum += weight;
            Node.weights.push_back(weight);
        }

        int counter = 0;

        for (std::set<int>::iterator it = Node.verticesIndices.begin(); it != Node.verticesIndices.end(); it++)
        {
            Node.weights[counter] = Node.weights[counter] / weightSum;
            counter++;
        }

        return true;
    }

    Node.subDivide(2.0, subDividedNodes);

    for (int i = 0; i < CellType::numberOfSubdivisions; i++)
    {
        //aabb[i].expand(0.2);
        subDividedNodes[i].expand(0.01);
    }

    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < CellType::numberOfSubdivisions; j++)
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

    int index;
    int parentLevel = (level-1);

    if (level == 0)
    {
        parentLevel = 0;
    }

    int offset = CellType::numberOfSubdivisions * (siblingIndex-levelStartIndex[parentLevel][0]);

    if (level == 0)
    {
        offset = 0;
    }

    for (int j = 0; j < CellType::numberOfSubdivisions; j++)
    {

        index = levelStartIndex[level][1] + offset + j;

        if (triangleMatrix[j].size() > 0)
        {
            treeAllLevels[index].copyShape(subDividedNodes[j]);
            treeAllLevels[index].level = level + 1;
            treeAllLevels[index].filled = true;
            createTree(treeAllLevels[index], triangleMatrix[j], level + 1, j + levelStartIndex[level][0] + offset);
        }
        else
        {
            treeAllLevels[index].filled = false;
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

    for (int i = levelStartIndex[maxLevel-1][0]; i < levelStartIndex[maxLevel-1][1]; i++)
    {

        current = &treeAllLevels[i];
        int nbrVertices = current->verticesIndices.size();
        smVec3f tempCenter(0, 0, 0);
        int counter = 0;

        if (current->filled)
        {
            for (std::set<int>::iterator it = current->verticesIndices.begin();
                    it != current->verticesIndices.end(); it++)
            {
                tempCenter = tempCenter + (mesh->vertices[*it]-mesh->origVerts[*it]) * current->weights[counter];
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
            i < levelStartIndex[maxLevel-1][1]; i++)
    {

        current = &treeAllLevels[i];
        initial = &initialTreeAllLevels[i];

        if (current->filled)
        {
            current->getCube().center = this->transRot.block(0,0,3,3) * initial->getCube().center + this->transRot.col(3).head(3);
        }
    }
}



#endif

