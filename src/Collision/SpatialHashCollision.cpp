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

#include "SpatialHashCollision.h"

// STL includes
#include <cmath>

// SimMedTK includes
#include "Core/CollisionConfig.h"
#include "Collision/SurfaceTree.h"
#include "Collision/OctreeCell.h"
#include "Collision/CollisionMoller.h"
#include "Core/SDK.h"


struct SpatialHashCollision::HashFunction
{
    HashFunction(unsigned int hashConst1 = 73856093,
                 unsigned int hashConst2 = 19349663,
                 unsigned int hashConst3 = 83492791) : const1(hashConst1),const2(hashConst2),const3(hashConst3)
                 {}

    unsigned int getKey(const unsigned int size, const unsigned int x, const unsigned int y, const unsigned int z)
    {
        return (((((x)* const1) ^ ((y)* const2) ^ ((z)* const3))) % (size));
    }
    unsigned int const1;
    unsigned int const2;
    unsigned int const3;
};

SpatialHashCollision::SpatialHashCollision(int hashTableSize,
                             float _cellSizeX,
                             float _cellSizeY,
                             float _cellSizeZ):
    cells(hashTableSize),
    cellLines(hashTableSize),
    cellsForTri2Line(hashTableSize),
    cellsForModel(hashTableSize),
    cellsForModelPoints(hashTableSize),
    hasher(make_unique<HashFunction>())
{
    cellSizeX = _cellSizeX;
    cellSizeY = _cellSizeY;
    cellSizeZ = _cellSizeZ;
}

SpatialHashCollision::~SpatialHashCollision()
{
}

void SpatialHashCollision::addCollisionModel(std::shared_ptr<SurfaceTreeType> CollMode)
{
    colModel.push_back(CollMode);
}

void SpatialHashCollision::addMesh(std::shared_ptr<Mesh> mesh)
{
    meshes.push_back(mesh);
    mesh->allocateAABBTris();
}

void SpatialHashCollision::addMesh(std::shared_ptr<LineMesh> mesh)
{
    lineMeshes.push_back(mesh);
}

void SpatialHashCollision::removeMesh(std::shared_ptr<Mesh> mesh)
{
    auto it = std::find(meshes.begin(),meshes.end(),mesh);
    if(it != meshes.end())
        meshes.erase(it);
}

bool SpatialHashCollision::findCandidatePoints(std::shared_ptr<Mesh> mesh,
                                          std::shared_ptr<SpatialHashCollision::SurfaceTreeType> colModel)
{
    AABB tempAABB;
    tempAABB.aabbMin = colModel->root->getCube().leftMinCorner();
    tempAABB.aabbMax = colModel->root->getCube().rightMaxCorner();

    bool  found = false;
    for (int i = 0; i < mesh->nbrVertices; i++)
    {
        if (CollisionMoller::checkAABBPoint(tempAABB, mesh->vertices[i]))
        {
            addPoint(mesh, i, cellsForModelPoints);
            found = true;
        }
    }
    return found;
}

bool SpatialHashCollision::findCandidateTris(std::shared_ptr<Mesh> meshA, std::shared_ptr<Mesh> meshB)
{
    AABB aabboverlap;

    if (CollisionMoller::checkOverlapAABBAABB(meshA->aabb, meshB->aabb, aabboverlap) == false)
    {
        return false;
    }

    for (int i = 0; i < meshA->nbrTriangles; i++)
    {
        addTriangle(meshA, i, cells);
    }

    for (int i = 0; i < meshB->nbrTriangles; i++)
    {
        addTriangle(meshB, i, cells);
    }

    return true;
}

bool SpatialHashCollision::findCandidateTrisLines(std::shared_ptr<Mesh> meshA, std::shared_ptr<LineMesh> meshB)
{
    AABB aabboverlap;

    if (CollisionMoller::checkOverlapAABBAABB(meshA->aabb, meshB->aabb, aabboverlap) == false)
    {
        return false;
    }

    for (int i = 0; i < meshA->nbrTriangles; i++)
    {
        if (CollisionMoller::checkOverlapAABBAABB(aabboverlap, meshA->triAABBs[i]))
        {
            addTriangle(meshA, i, cellsForTri2Line);
        }
    }

    for (int i = 0; i < meshB->nbrEdges; i++)
    {
        if (CollisionMoller::checkOverlapAABBAABB(aabboverlap, meshB->edgeAABBs[i]))
        {
            addLine(meshB, i, cellLines);
        }
    }

    return true;
}

void SpatialHashCollision::computeCollisionTri2Tri()
{
    smHashIterator<CellTriangle> iterator;
    smHashIterator<CellTriangle> iterator1;

    CellTriangle triA;
    CellTriangle triB;
    core::Vec3d proj1, proj2, inter1, inter2;
    short point1, point2;
    int coPlanar;

    while (cells.next(iterator))
    {
        while (cells.nextBucketItem(iterator, triA))
        {
            iterator1.clone(iterator);

            while (cells.nextBucketItem(iterator1, triB))
            {
                if (triA.meshID == triB.meshID ||
                    !(meshes[0]->collisionGroup.isCollisionPermitted(meshes[1]->collisionGroup)))
                {
                    continue;
                }

                if (CollisionMoller::tri2tri(triA.vert[0],
                                               triA.vert[1],
                                               triA.vert[2],
                                               triB.vert[0],
                                               triB.vert[1],
                                               triB.vert[2],
                                               coPlanar,
                                               inter1,
                                               inter2,
                                               point1,
                                               point2,
                                               proj1,
                                               proj2))
                {
                    auto collisionPair = std::make_shared<smCollidedTriangles>();
                    collisionPair->tri1 = triA;
                    collisionPair->tri2 = triB;
                    collidedTriangles.push_back(collisionPair);
                }
            }
        }
    }
}

///line to triangle collision
void  SpatialHashCollision::computeCollisionLine2Tri()
{
    smHashIterator<smCellLine > iteratorLine;
    smHashIterator<CellTriangle > iteratorTri;
    smCellLine line;
    CellTriangle tri;
    core::Vec3d intersection;

    while (cellLines.next(iteratorLine) && cellsForTri2Line.next(iteratorTri))
    {
        while (cellLines.nextBucketItem(iteratorLine, line))
        {

            iteratorTri.resetBucketIteration();

            while (cellsForTri2Line.nextBucketItem(iteratorTri, tri))
            {
                if (tri.meshID == line.meshID ||
                    !(meshes[0]->collisionGroup.isCollisionPermitted(meshes[1]->collisionGroup)))
                {
                    continue;
                }

                if (CollisionMoller::checkLineTri(line.vert[0],
                                                    line.vert[1],
                                                    tri.vert[0],
                                                    tri.vert[1],
                                                    tri.vert[2],
                                                    intersection))
                {
                    auto collisionPair = std::make_shared<smCollidedLineTris>();
                    collisionPair->line = line;
                    collisionPair->tri = tri;
                    collisionPair->intersection = intersection;
                    collidedLineTris.push_back(collisionPair);
                }
            }
        }
    }
}

void SpatialHashCollision::computeCollisionModel2Points()
{
    smHashIterator<smCellModel > iteratorModel;
    smHashIterator<smCellPoint > iteratorPoint;
    smCellModel model;
    smCellPoint point;

    while (cellsForModel.next(iteratorModel) && cellsForModelPoints.next(iteratorPoint))
    {
        while (cellsForModel.nextBucketItem(iteratorModel, model))
        {

            iteratorPoint.resetBucketIteration();

            while (cellsForModelPoints.nextBucketItem(iteratorPoint, point))
            {

                float distanceFromCenter = (model.center - point.vert).norm();

                if (distanceFromCenter < model.radius)
                {
                    auto collisionPair = std::make_shared<smCollidedModelPoints>();
                    collisionPair->penetration = model.radius - distanceFromCenter;
                    collisionPair->model = model;
                    collisionPair->point = point;
                    collidedModelPoints.push_back(collisionPair);
                }
            }
        }
    }
}

void SpatialHashCollision::computeHash(std::shared_ptr<Mesh> mesh, const std::vector<int> &triangleIndexes)
{
    for(auto&& i : triangleIndexes)
    {
        int xStartIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMin[0]/cellSizeX));
        int yStartIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMin[1]/cellSizeY));
        int zStartIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMin[2]/cellSizeZ));

        int xEndIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMax[0]/cellSizeX));
        int yEndIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMax[1]/cellSizeY));
        int zEndIndex = static_cast<int>(std::floor(mesh->triAABBs[i].aabbMax[2]/cellSizeZ));

        for (int ix = xStartIndex; ix <= xEndIndex; ix++)
            for (int iy = yStartIndex; iy <= yEndIndex; iy++)
                for (int iz = zStartIndex; iz <= zEndIndex; iz++)
                {
                    cells.insert(CellTriangle(i), hasher->getKey(cells.tableSize, ix, iy, iz));
                }
    }
}

void SpatialHashCollision::addTriangle(std::shared_ptr<Mesh> mesh, int triangleId, smHash<CellTriangle> &cells)
{
    CellTriangle  triangle;
    triangle.meshID = mesh->getUniqueId();
    triangle.primID = triangleId;

    triangle.vert[0] = mesh->vertices[mesh->triangles[triangleId].vert[0]];
    triangle.vert[1] = mesh->vertices[mesh->triangles[triangleId].vert[1]];
    triangle.vert[2] = mesh->vertices[mesh->triangles[triangleId].vert[2]];

    int xStartIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMin[0]/cellSizeX));
    int yStartIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMin[1]/cellSizeY));
    int zStartIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMin[2]/cellSizeZ));

    int xEndIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMax[0]/cellSizeX));
    int yEndIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMax[1]/cellSizeY));
    int zEndIndex = static_cast<int>(std::floor(mesh->triAABBs[triangleId].aabbMax[2]/cellSizeZ));

    for (int ix = xStartIndex; ix <= xEndIndex; ix++)
        for (int iy = yStartIndex; iy <= yEndIndex; iy++)
            for (int iz = zStartIndex; iz <= zEndIndex; iz++)
            {
                cells.checkAndInsert(triangle, hasher->getKey(cells.tableSize, ix, iy, iz));
            }
}

void SpatialHashCollision::addLine(std::shared_ptr<LineMesh> mesh,
                                   int edgeId, smHash<smCellLine> &cells)
{
    smCellLine  line;
    line.meshID = mesh->getUniqueId();
    line.primID = edgeId;
    line.vert[0] = mesh->vertices[mesh->edges[edgeId].vert[0]];
    line.vert[1] = mesh->vertices[mesh->edges[edgeId].vert[1]];

    int xStartIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMin[0]/cellSizeX));
    int yStartIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMin[1]/cellSizeY));
    int zStartIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMin[2]/cellSizeZ));

    int xEndIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMax[0]/cellSizeX));
    int yEndIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMax[1]/cellSizeY));
    int zEndIndex = static_cast<int>(std::floor(mesh->edgeAABBs[edgeId].aabbMax[2]/cellSizeZ));

    for (int ix = xStartIndex; ix <= xEndIndex; ix++)
        for (int iy = yStartIndex; iy <= yEndIndex; iy++)
            for (int iz = zStartIndex; iz <= zEndIndex; iz++)
            {
                cells.checkAndInsert(line, hasher->getKey(cells.tableSize, ix, iy, iz));
            }
}

void SpatialHashCollision::addPoint(std::shared_ptr<Mesh> mesh, int vertId, smHash<smCellPoint> &cells)
{
    smCellPoint cellPoint;
    cellPoint.meshID = mesh->getUniqueId();
    cellPoint.primID = vertId;
    cellPoint.vert = mesh->vertices[vertId];

    int xStartIndex = static_cast<int>(std::floor(mesh->vertices[vertId][0]/cellSizeX));
    int yStartIndex = static_cast<int>(std::floor(mesh->vertices[vertId][1]/cellSizeY));
    int zStartIndex = static_cast<int>(std::floor(mesh->vertices[vertId][2]/cellSizeZ));

    cells.checkAndInsert(cellPoint, hasher->getKey(cells.tableSize, xStartIndex, yStartIndex, zStartIndex));
}

void SpatialHashCollision::addOctreeCell(std::shared_ptr<SpatialHashCollision::SurfaceTreeType> colModel, smHash<smCellModel> &cells)
{
    smCellModel cellModel;
    AABB temp;

    SurfaceTreeIterator<OctreeCell> iter = colModel->getLevelIterator();
    cellModel.meshID = colModel->getAttachedMeshID();

    for (int i = iter.start(); i != iter.end(); ++i)
    {
        if (!iter[i].isEmpty())
        {
            temp.aabbMin =  iter[i].getCube().leftMinCorner();
            temp.aabbMax =  iter[i].getCube().rightMaxCorner();

            int xStartIndex = static_cast<int>(std::floor(temp.aabbMin[0]/cellSizeX));
            int yStartIndex = static_cast<int>(std::floor(temp.aabbMin[1]/cellSizeY));
            int zStartIndex = static_cast<int>(std::floor(temp.aabbMin[2]/cellSizeZ));

            int xEndIndex = static_cast<int>(std::floor(temp.aabbMin[0]/cellSizeX));
            int yEndIndex = static_cast<int>(std::floor(temp.aabbMin[1]/cellSizeY));
            int zEndIndex = static_cast<int>(std::floor(temp.aabbMin[2]/cellSizeZ));

            cellModel.primID = i;
            cellModel.center = iter[i].getCube().center;
            cellModel.radius = iter[i].getCube().getCircumscribedSphere().getRadius();

            for (int ix = xStartIndex; ix <= xEndIndex; ix++)
                for (int iy = yStartIndex; iy <= yEndIndex; iy++)
                    for (int iz = zStartIndex; iz <= zEndIndex; iz++)
                    {
                        cells.checkAndInsert(cellModel, hasher->getKey(cells.tableSize, ix, iy, iz));
                    }
        }
    }
}

void SpatialHashCollision::reset()
{
    cells.clearAll();
    cellLines.clearAll();
    cellsForTri2Line.clearAll();
    cellsForModelPoints.clearAll();
    lineMeshes.clear();
    collidedLineTris.clear();
    collidedModelPoints.clear();
    collidedTriangles.clear();
}
bool SpatialHashCollision::findCandidates()
{
    for(size_t i = 0; i < colModel.size(); i++)
        for(size_t i = 0; i < meshes.size(); i++)
        {
            findCandidatePoints(meshes[i], colModel[i]);
            addOctreeCell(colModel[i], cellsForModel);
        }

    ///Triangle-Triangle collision
    for(size_t i = 0; i < meshes.size(); i++)
    {
        for(size_t j = i + 1; j < meshes.size(); j++)
        {
            if(meshes[i]->collisionGroup.isCollisionPermitted(meshes[j]->collisionGroup))
            {
                if(findCandidateTris(meshes[i], meshes[j]) == false)
                {
                    continue;
                }
            }
        }
    }

    ///Triangle-line Collision
    for(size_t i = 0; i < meshes.size(); i++)
        for(size_t j = 0; j < lineMeshes.size(); j++)
        {
            if(meshes[i]->collisionGroup.isCollisionPermitted(lineMeshes[j]->collisionGroup))
            {
                if(findCandidateTrisLines(meshes[i], lineMeshes[j]) == false)
                {
                    continue;
                }
            }
        }
    return 0;
}
void SpatialHashCollision::updateBVH()
{
    for(size_t i = 0; i < meshes.size(); i++)
    {
        meshes[i]->updateTriangleAABB();
    }

    for(size_t i = 0; i < lineMeshes.size(); i++)
    {
        meshes[i]->upadateAABB();
    }
}
const std::vector< std::shared_ptr< smCollidedTriangles > >& SpatialHashCollision::getCollidedTriangles() const
{
    return collidedTriangles;
}
std::vector< std::shared_ptr< smCollidedTriangles > >& SpatialHashCollision::getCollidedTriangles()
{
    return collidedTriangles;
}
