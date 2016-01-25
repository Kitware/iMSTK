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

#include "SpatialHashCollision.h"

#include <Eigen/Geometry>

// iMSTK includes
#include "Core/CollisionConfig.h"
#include "Collision/SurfaceTree.h"
#include "Collision/OctreeCell.h"
#include "Collision/CollisionMoller.h"
#include "Collision/MeshCollisionModel.h"
#include "Mesh/SurfaceMesh.h"


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
    hasher(Core::make_unique<HashFunction>())
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

void SpatialHashCollision::addModel(std::shared_ptr<MeshCollisionModel> model)
{
    this->collisionModels.push_back(model);
}

void SpatialHashCollision::removeModel(std::shared_ptr<MeshCollisionModel> model)
{
    auto it = std::find(this->collisionModels.begin(),this->collisionModels.end(),model);
    if(it != this->collisionModels.end())
        this->collisionModels.erase(it);
}

bool SpatialHashCollision::findCandidatePoints(std::shared_ptr<MeshCollisionModel> model,
                                          std::shared_ptr<SpatialHashCollision::SurfaceTreeType> tree)
{
    AABB tempAABB;
    tempAABB.aabbMin = tree->root->getCube().leftMinCorner();
    tempAABB.aabbMax = tree->root->getCube().rightMaxCorner();

    bool  found = false;

    auto &vertices = model->getVertices();
    for (size_t i = 0, end = vertices.size(); i < end; ++i)
    {
        if (CollisionMoller::checkAABBPoint(tempAABB, vertices[i]))
        {
            addPoint(model, i, cellsForModelPoints);
            found = true;
        }
    }
    return found;
}

bool SpatialHashCollision::findCandidateTris(std::shared_ptr<MeshCollisionModel> modelA, std::shared_ptr<MeshCollisionModel> modelB)
{
    auto intersection = modelA->getBoundingBox().intersection(modelB->getBoundingBox());
    if (intersection.isEmpty())
    {
        return false;
    }

    for (size_t i = 0, end = modelA->getTriangles().size(); i < end; i++)
    {
        addTriangle(modelA, i, cells);
    }

    for (size_t i = 0, end = modelB->getTriangles().size(); i < end; i++)
    {
        addTriangle(modelB, i, cells);
    }

    return true;
}

void SpatialHashCollision::computeCollisionTri2Tri()
{
    HashIterator<CellTriangle> iterator;
    HashIterator<CellTriangle> iterator1;

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
                    !(collisionModels[0]->getCollisionGroup()->isCollisionPermitted(collisionModels[1]->getCollisionGroup())))
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
                    auto collisionPair = std::make_shared<CollidedTriangles>();
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
    HashIterator<CellLine > iteratorLine;
    HashIterator<CellTriangle > iteratorTri;
    CellLine line;
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
                    !(collisionModels[0]->getCollisionGroup()->isCollisionPermitted(collisionModels[1]->getCollisionGroup())))
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
                    auto collisionPair = std::make_shared<CollidedLineTris>();
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
    HashIterator<CellModel > iteratorModel;
    HashIterator<CellPoint > iteratorPoint;
    CellModel model;
    CellPoint point;

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
                    auto collisionPair = std::make_shared<CollidedModelPoints>();
                    collisionPair->penetration = model.radius - distanceFromCenter;
                    collisionPair->model = model;
                    collisionPair->point = point;
                    collidedModelPoints.push_back(collisionPair);
                }
            }
        }
    }
}

void SpatialHashCollision::computeHash(std::shared_ptr<MeshCollisionModel> model, const std::vector<int> &triangleIndexes)
{
    core::Vec3d cellSize(1.0/cellSizeX,1.0/cellSizeY,1.0/cellSizeZ);
    for(auto&& i : triangleIndexes)
    {
        auto startIndices = model->getAabb(i).min().array()*cellSize.array();
        auto endIndices = model->getAabb(i).max().array()*cellSize.array();

        int xStartIndex = static_cast<int>(std::floor(startIndices(0)));
        int yStartIndex = static_cast<int>(std::floor(startIndices(1)));
        int zStartIndex = static_cast<int>(std::floor(startIndices(2)));

        int xEndIndex = static_cast<int>(std::floor(endIndices(0)));
        int yEndIndex = static_cast<int>(std::floor(endIndices(1)));
        int zEndIndex = static_cast<int>(std::floor(endIndices(2)));

        for (int ix = xStartIndex; ix <= xEndIndex; ix++)
            for (int iy = yStartIndex; iy <= yEndIndex; iy++)
                for (int iz = zStartIndex; iz <= zEndIndex; iz++)
                {
                    cells.insert(CellTriangle(i), hasher->getKey(cells.tableSize, ix, iy, iz));
                }
    }
}

void SpatialHashCollision::addTriangle(std::shared_ptr<MeshCollisionModel> model, int triangleId, Hash<CellTriangle> &cells)
{
    CellTriangle  triangle;
    triangle.meshID = model->getMesh()->getUniqueId();
    triangle.primID = triangleId;

    core::Vec3d cellSize(1.0/cellSizeX,1.0/cellSizeY,1.0/cellSizeZ);

    auto startIndices = model->getAabb(triangleId).min().array()*cellSize.array();
    auto endIndices = model->getAabb(triangleId).max().array()*cellSize.array();

    int xStartIndex = static_cast<int>(std::floor(startIndices(0)));
    int yStartIndex = static_cast<int>(std::floor(startIndices(1)));
    int zStartIndex = static_cast<int>(std::floor(startIndices(2)));

    int xEndIndex = static_cast<int>(std::floor(endIndices(0)));
    int yEndIndex = static_cast<int>(std::floor(endIndices(1)));
    int zEndIndex = static_cast<int>(std::floor(endIndices(2)));

    auto &vertices = model->getVertices();
    auto &triangles = model->getTriangles();
    triangle.vert[0] = vertices[triangles[triangleId][0]];
    triangle.vert[1] = vertices[triangles[triangleId][1]];
    triangle.vert[2] = vertices[triangles[triangleId][2]];
    for (int ix = xStartIndex; ix <= xEndIndex; ix++)
        for (int iy = yStartIndex; iy <= yEndIndex; iy++)
            for (int iz = zStartIndex; iz <= zEndIndex; iz++)
            {
                cells.checkAndInsert(triangle, hasher->getKey(cells.tableSize, ix, iy, iz));
            }
}


void SpatialHashCollision::addPoint(std::shared_ptr<MeshCollisionModel> model, int vertId, Hash<CellPoint> &cells)
{
    CellPoint cellPoint;
    cellPoint.meshID = model->getMesh()->getUniqueId();
    cellPoint.primID = vertId;

    auto &vertices = model->getVertices();

    cellPoint.vert = vertices[vertId];

    int xStartIndex = static_cast<int>(std::floor(vertices[vertId][0]/cellSizeX));
    int yStartIndex = static_cast<int>(std::floor(vertices[vertId][1]/cellSizeY));
    int zStartIndex = static_cast<int>(std::floor(vertices[vertId][2]/cellSizeZ));

    cells.checkAndInsert(cellPoint, hasher->getKey(cells.tableSize, xStartIndex, yStartIndex, zStartIndex));
}

void SpatialHashCollision::addOctreeCell(std::shared_ptr<SpatialHashCollision::SurfaceTreeType> colModel, Hash<CellModel> &cells)
{
    CellModel cellModel;
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
    collidedLineTris.clear();
    collidedModelPoints.clear();
    collidedTriangles.clear();
}
bool SpatialHashCollision::findCandidates()
{
    for(size_t i = 0; i < colModel.size(); i++)
    {
        for(size_t i = 0; i < this->collisionModels.size(); i++)
        {
            findCandidatePoints(collisionModels[i], colModel[i]);
            addOctreeCell(colModel[i], cellsForModel);
        }
    }

    ///Triangle-Triangle collision
    for(size_t i = 0; i < collisionModels.size(); i++)
    {
        for(size_t j = i + 1; j < collisionModels.size(); j++)
        {
            if(findCandidateTris(collisionModels[i], collisionModels[j]) == false &&
               collisionModels[i]->getCollisionGroup()->isCollisionPermitted(collisionModels[j]->getCollisionGroup()) !=
               NULL)
            {
                continue;
            }
        }
    }

    return 0;
}
void SpatialHashCollision::updateBVH()
{
    for(size_t i = 0; i < collisionModels.size(); i++)
    {
        collisionModels[i]->updateBoundingBoxes();
    }
}
const std::vector< std::shared_ptr< CollidedTriangles > >& SpatialHashCollision::getCollidedTriangles() const
{
    return collidedTriangles;
}
std::vector< std::shared_ptr< CollidedTriangles > >& SpatialHashCollision::getCollidedTriangles()
{
    return collidedTriangles;
}
