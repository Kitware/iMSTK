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

#ifndef COLLISION_SPATIALHASH_H
#define COLLISION_SPATIALHASH_H

// STD includes
#include <memory>

// iMSTK includes
#include "Core/CollisionDetection.h"
#include "Core/DataStructures.h"
#include "Core/MakeUnique.h"

class CellLine;
class CellModel;
class CellPoint;
class CellTriangle;
class CollidedLineTris;
class CollidedModelPoints;
class CollidedTriangles;
class SurfaceMesh;
class MeshCollisionModel;
class OctreeCell;

template<typename CellType>
class SurfaceTree;

/// \brief spatial hash
class SpatialHashCollision: public CollisionDetection
{
public:
    using SurfaceTreeType = SurfaceTree<OctreeCell>;

public:

    /// \brief !!
    SpatialHashCollision(int hashTableSize,
                  float cellSizeX,
                  float cellSizeY,
                  float cellSizeZ);

    /// \brief destructor clear some memory
    ~SpatialHashCollision();

    /// \brief !!
    void addCollisionModel(std::shared_ptr<SurfaceTreeType> CollModel);

    /// \brief !!
    void addModel(std::shared_ptr<MeshCollisionModel> mesh);

    /// \brief !!
    void removeModel(std::shared_ptr<MeshCollisionModel> mesh);

    /// \brief !!
    bool findCandidates(/*std::shared_ptr<SurfaceMesh> meshA, std::shared_ptr<SurfaceMesh> meshB*/);

    /// \brief !!
    bool findCandidatePoints(std::shared_ptr<MeshCollisionModel> model, std::shared_ptr<SurfaceTreeType> tree);

    /// \brief find the candidate triangle pairs for collision (broad phase collision)
    bool findCandidateTris(std::shared_ptr<MeshCollisionModel> modelA, std::shared_ptr<MeshCollisionModel> ModelB);

    /// \brief compute the collision between two triangles (narrow phase collision)
    void computeCollisionTri2Tri();

    /// \brief compute collision between a line segment and a triangle (narrow phase collision)
    void computeCollisionLine2Tri();

    /// \brief !!
    void computeCollisionModel2Points();

    /// \brief !! compute the hash
    void computeHash(std::shared_ptr<MeshCollisionModel> model, const std::vector<int> &tris);

    const std::vector<std::shared_ptr<CollidedTriangles>> &getCollidedTriangles() const;

    std::vector<std::shared_ptr<CollidedTriangles>> &getCollidedTriangles();

protected:
    /// \brief adds triangle to hash
    void addTriangle(std::shared_ptr<MeshCollisionModel> model, int triangleId, Hash<CellTriangle> &cells);

    /// \brief adds point to hash
    void addPoint(std::shared_ptr<MeshCollisionModel> model, int vertId, Hash<CellPoint> &cells);

    /// \brief adds octree cell to hash
    void addOctreeCell(std::shared_ptr<SurfaceTreeType> colModel, Hash<CellModel> &cells);

    /// \brief !!
    void reset();

    void updateBVH();

private:
    void doComputeCollision(std::shared_ptr<CollisionManager> /*pairs*/)
    {
        reset();
        updateBVH();
        findCandidates();
        computeCollisionTri2Tri();
//         computeCollisionLine2Tri();
//         computeCollisionModel2Points();
    }

private:
    float cellSizeX; ///< cell spacing in x-direction
    float cellSizeY; ///< cell spacing in y-direction
    float cellSizeZ; ///< cell spacing in z-direction

    Hash<CellTriangle> cells; // Candidate triangles
    Hash<CellLine> cellLines; // Lines that stored in the scene.
    Hash<CellTriangle> cellsForTri2Line;  // Candidate triangles in the scene.
    Hash<CellModel> cellsForModel; // Candidate cells for collision model
    Hash<CellPoint> cellsForModelPoints; // Candidate for Collision model to point
    std::vector<std::shared_ptr<MeshCollisionModel>> collisionModels; // SurfaceMesh models
    std::vector<std::shared_ptr<CollidedTriangles>> collidedTriangles; // List of collision pairs triangles
    std::vector<std::shared_ptr<CollidedLineTris>> collidedLineTris; // List of collision pairs triangles-lines
    std::vector<std::shared_ptr<CollidedModelPoints>> collidedModelPoints; // List of collision pairs models-points
    std::vector<std::shared_ptr<SurfaceTreeType>> colModel;
    struct HashFunction;
    std::unique_ptr<HashFunction> hasher;
};

#endif
