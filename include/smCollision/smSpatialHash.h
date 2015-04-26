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
#include "smCollision/smCollisionDetection.h"
#include "smUtilities/smDataStructures.h"
#include "smUtilities/smMakeUnique.h"

class smCellLine;
class smCellModel;
class smCellPoint;
class smCellTriangle;
class smCollidedLineTris;
class smCollidedModelPoints;
class smCollidedTriangles;
class smLineMesh;
class smMesh;
class smOctreeCell;

template<typename CellType>
class smSurfaceTree;

/// \brief
enum smCollisionSetting
{
    SIMMEDTK_COLISIONSETTING_MODEL2POINT,
    SIMMEDTK_COLISIONSETTING_MODEL2TRIANGLE,
    SIMMEDTK_COLISIONSETTING_MODEL2MODEL
};

/// \brief spatial hash
class smSpatialHash: public smCollisionDetection
{
public:
    using SurfaceTreeType = smSurfaceTree<smOctreeCell>;

public:

    /// \brief !!
    smSpatialHash(smInt hashTableSize,
                  smFloat cellSizeX,
                  smFloat cellSizeY,
                  smFloat cellSizeZ);

    /// \brief destructor clear some memory
    ~smSpatialHash();

    /// \brief !!
    void addCollisionModel(std::shared_ptr<SurfaceTreeType> CollModel);

    /// \brief !!
    void addMesh(std::shared_ptr<smMesh> mesh);

    /// \brief !!
    void addMesh(std::shared_ptr<smLineMesh> mesh);

    /// \brief !!
    void removeMesh(std::shared_ptr<smMesh> mesh);

    /// \brief !!
    smBool findCandidates(/*std::shared_ptr<smMesh> meshA, std::shared_ptr<smMesh> meshB*/);

    /// \brief !!
    smBool findCandidatePoints(std::shared_ptr<smMesh> mesh, std::shared_ptr<SurfaceTreeType> colModel);

    /// \brief find the candidate triangle pairs for collision (broad phase collision)
    smBool  findCandidateTris(std::shared_ptr<smMesh> meshA, std::shared_ptr<smMesh> meshB);

    /// \brief find the candidate line-triangle pairs for collision (broad phase collision)
    smBool  findCandidateTrisLines(std::shared_ptr<smMesh> meshA, std::shared_ptr<smLineMesh> meshB);

    /// \brief compute the collision between two triangles (narrow phase collision)
    void computeCollisionTri2Tri();

    /// \brief compute collision between a line segment and a triangle (narrow phase collision)
    void computeCollisionLine2Tri();

    /// \brief !!
    void computeCollisionModel2Points();

    /// \brief !! compute the hash
    void computeHash(std::shared_ptr<smMesh> mesh, const std::vector<smInt> &tris);

    const std::vector<std::shared_ptr<smCollidedTriangles>> &getCollidedTriangles() const;

    std::vector<std::shared_ptr<smCollidedTriangles>> &getCollidedTriangles();

protected:
    /// \brief adds triangle to hash
    void addTriangle(std::shared_ptr<smMesh> mesh, int triangleId, smHash<smCellTriangle> &cells);

    /// \brief adds line to hash
    void addLine(std::shared_ptr<smLineMesh> mesh, int edgeId, smHash<smCellLine> &cells);

    /// \brief adds point to hash
    void addPoint(std::shared_ptr<smMesh> mesh, int vertId, smHash<smCellPoint> &cells);

    /// \brief adds octree cell to hash
    void addOctreeCell(std::shared_ptr<SurfaceTreeType> colModel, smHash<smCellModel> &cells);

    /// \brief !!
    void reset();

    void updateBVH();

private:
    void doComputeCollision(std::shared_ptr<smCollisionPair> /*pairs*/)
    {
        reset();
        updateBVH();
        findCandidates();
        computeCollisionTri2Tri();
        computeCollisionLine2Tri();
        computeCollisionModel2Points();
    }

private:
    smFloat cellSizeX; ///< cell spacing in x-direction
    smFloat cellSizeY; ///< cell spacing in y-direction
    smFloat cellSizeZ; ///< cell spacing in z-direction

    smHash<smCellTriangle> cells; // Candidate triangles
    smHash<smCellLine> cellLines; // Lines that stored in the scene.
    smHash<smCellTriangle> cellsForTri2Line;  // Candidate triangles in the scene.
    smHash<smCellModel> cellsForModel; // Candidate cells for collision model
    smHash<smCellPoint> cellsForModelPoints; // Candidate for Collision model to point
    std::vector<std::shared_ptr<smMesh>> meshes; // Mesh models
    std::vector<std::shared_ptr<smLineMesh>> lineMeshes; // Line mehs models
    std::vector<std::shared_ptr<smCollidedTriangles>> collidedTriangles; // List of collision pairs triangles
    std::vector<std::shared_ptr<smCollidedLineTris>> collidedLineTris; // List of collision pairs triangles-lines
    std::vector<std::shared_ptr<smCollidedModelPoints>> collidedModelPoints; // List of collision pairs models-points
    std::vector<std::shared_ptr<SurfaceTreeType>> colModel;
    struct HashFunction;
    std::unique_ptr<HashFunction> hasher;
};

#endif
