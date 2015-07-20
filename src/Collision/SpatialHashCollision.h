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

#ifndef SM_SPATIALHASH_H
#define SM_SPATIALHASH_H

// STD includes
#include <memory>

// SimMedTK includes
#include "Core/CollisionDetection.h"
#include "Core/DataStructures.h"
#include "Core/MakeUnique.h"

class smCellLine;
class smCellModel;
class smCellPoint;
class CellTriangle;
class smCollidedLineTris;
class smCollidedModelPoints;
class smCollidedTriangles;
class LineMesh;
class Mesh;
class OctreeCell;

template<typename CellType>
class SurfaceTree;

/// \brief
enum smCollisionSetting
{
    SIMMEDTK_COLISIONSETTING_MODEL2POINT,
    SIMMEDTK_COLISIONSETTING_MODEL2TRIANGLE,
    SIMMEDTK_COLISIONSETTING_MODEL2MODEL
};

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
    void addMesh(std::shared_ptr<Mesh> mesh);

    /// \brief !!
    void addMesh(std::shared_ptr<LineMesh> mesh);

    /// \brief !!
    void removeMesh(std::shared_ptr<Mesh> mesh);

    /// \brief !!
    bool findCandidates(/*std::shared_ptr<Mesh> meshA, std::shared_ptr<Mesh> meshB*/);

    /// \brief !!
    bool findCandidatePoints(std::shared_ptr<Mesh> mesh, std::shared_ptr<SurfaceTreeType> colModel);

    /// \brief find the candidate triangle pairs for collision (broad phase collision)
    bool findCandidateTris(std::shared_ptr<Mesh> meshA, std::shared_ptr<Mesh> meshB);

    /// \brief find the candidate line-triangle pairs for collision (broad phase collision)
    bool findCandidateTrisLines(std::shared_ptr<Mesh> meshA, std::shared_ptr<LineMesh> meshB);

    /// \brief compute the collision between two triangles (narrow phase collision)
    void computeCollisionTri2Tri();

    /// \brief compute collision between a line segment and a triangle (narrow phase collision)
    void computeCollisionLine2Tri();

    /// \brief !!
    void computeCollisionModel2Points();

    /// \brief !! compute the hash
    void computeHash(std::shared_ptr<Mesh> mesh, const std::vector<int> &tris);

    const std::vector<std::shared_ptr<smCollidedTriangles>> &getCollidedTriangles() const;

    std::vector<std::shared_ptr<smCollidedTriangles>> &getCollidedTriangles();

protected:
    /// \brief adds triangle to hash
    void addTriangle(std::shared_ptr<Mesh> mesh, int triangleId, smHash<CellTriangle> &cells);

    /// \brief adds line to hash
    void addLine(std::shared_ptr<LineMesh> mesh, int edgeId, smHash<smCellLine> &cells);

    /// \brief adds point to hash
    void addPoint(std::shared_ptr<Mesh> mesh, int vertId, smHash<smCellPoint> &cells);

    /// \brief adds octree cell to hash
    void addOctreeCell(std::shared_ptr<SurfaceTreeType> colModel, smHash<smCellModel> &cells);

    /// \brief !!
    void reset();

    void updateBVH();

private:
    void doComputeCollision(std::shared_ptr<CollisionPair> /*pairs*/)
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

    smHash<CellTriangle> cells; // Candidate triangles
    smHash<smCellLine> cellLines; // Lines that stored in the scene.
    smHash<CellTriangle> cellsForTri2Line;  // Candidate triangles in the scene.
    smHash<smCellModel> cellsForModel; // Candidate cells for collision model
    smHash<smCellPoint> cellsForModelPoints; // Candidate for Collision model to point
    std::vector<std::shared_ptr<Mesh>> meshes; // Mesh models
    std::vector<std::shared_ptr<LineMesh>> lineMeshes; // Line mehs models
    std::vector<std::shared_ptr<smCollidedTriangles>> collidedTriangles; // List of collision pairs triangles
    std::vector<std::shared_ptr<smCollidedLineTris>> collidedLineTris; // List of collision pairs triangles-lines
    std::vector<std::shared_ptr<smCollidedModelPoints>> collidedModelPoints; // List of collision pairs models-points
    std::vector<std::shared_ptr<SurfaceTreeType>> colModel;
    struct HashFunction;
    std::unique_ptr<HashFunction> hasher;
};

#endif
