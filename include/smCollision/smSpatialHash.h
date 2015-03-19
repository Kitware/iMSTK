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
#include <map>

// SimMedTK includes
#include "smCore/smObjectSimulator.h"
#include "smUtilities/smDataStructures.h"

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
class smPipe;

template<typename CellType>
class smSurfaceTree;

/// \brief
enum smCollisionSetting
{
    SIMMEDTK_COLISIONSETTING_MODEL2POINT,
    SIMMEDTK_COLISIONSETTING_MODEL2TRIANGLE,
    SIMMEDTK_COLISIONSETTING_MODEL2MODEL
};

#define SIMMEDTK_COLLISIONOUTPUTBUF_SIZE 1000
#define HASH_P1 73856093
#define HASH_P2 19349663
#define HASH_P3 83492791

unsigned int HASH(unsigned int SIZE, unsigned int x, unsigned int y, unsigned int z);

/// \brief spatial hash
class smSpatialHash: public smObjectSimulator
{

protected:
    float cellSizeX; ///< cell size in x-direction
    float cellSizeY; ///< cell size in y-direction
    float cellSizeZ; ///< cell size in z-direction

    //These structures below are Triangle2Triangle collisions

    /// \brief Cells for triangles. It stores candidate triangles
    smHash<smCellTriangle> cells;

    /// \brief structure that stores the meshes in the scene
    std::vector<smMesh *> meshes;

    /// \brief After the collision is cimpleted the result is written in here
    smCollidedTriangles *collidedPrims;

    /// \brief Number of collisions that triangles are stored.
    int nbrTriCollisions;

    /// \brief Line mesh structure that is added to collision detection engine
    std::vector<smLineMesh *> lineMeshes;

    /// \brief Lines that stored in the scene.
    smHash<smCellLine> cellLines;

    /// \brief candidate triangles in the scene.
    smHash<smCellTriangle> cellsForTri2Line;

    /// \brief candidate cells for collision model
    smHash<smCellModel> cellsForModel;

    /// \brief candidate for Collision model to point
    smHash<smCellPoint> cellsForModelPoints;

    /// \brief The result is stored here.
    smCollidedLineTris *collidedLineTris;

    /// \brief The number of collisions that for line to triangle.
    int nbrLineTriCollisions;

    /// \brief smHash<smCollisionGroup> collisionGroups;

    /// \brief the collision results are here
    smCollidedModelPoints *collidedModelPoints;

    /// \brief the number of collisions for model to point
    int nbrModelPointCollisions;

    /// \brief For maximum collision output.
    int maxPrims;

    std::map<int, int> filteredList;

    /// \brief adds triangle to hash
    void addTriangle(smMesh *mesh, int triangleId, smHash<smCellTriangle> &cells);

    /// \brief adds line to hash
    void addLine(smLineMesh *mesh, int edgeId, smHash<smCellLine> &cells);

    /// \brief adds point to hash
    void addPoint(smMesh *mesh, int vertId, smHash<smCellPoint> cells);

    /// \brief adds octree cell to hash
    void addOctreeCell(smSurfaceTree<smOctreeCell> *colModel, smHash<smCellModel> cells);

    /// \brief !!
    void reset();

    std::vector<smSurfaceTree<smOctreeCell>*> colModel;

public:
    bool enableDuplicateFilter; ///< !!
    smPipe *pipe; ///< !!
    smPipe *pipeTriangles; ///< !!
    smPipe *pipeModelPoints; ///< !!

    /// \brief !!
    void  findCandidatePoints(smMesh *mesh, smSurfaceTree<smOctreeCell> *colModel);

    /// \brief !!
    void computeCollisionModel2Points();

    /// \brief destructor clear some memory
    ~smSpatialHash();

    /// \brief !!
    void addCollisionModel(smSurfaceTree<smOctreeCell> *CollMode);

    /// \brief !!
    void addMesh(smMesh *mesh);

    /// \brief !!
    void addMesh(smLineMesh *mesh);

    /// \brief !!
    void removeMesh(smMesh *mesh);

    /// \brief !!
    smSpatialHash(smErrorLog *errorLog,
                  int hashTableSize,
                  float cellSizeX,
                  float cellSizeY,
                  float cellSizeZ,
                  int outOutputPrimSize = SIMMEDTK_COLLISIONOUTPUTBUF_SIZE);

    /// \brief !!
    void draw();

    /// \brief find the candidate triangle pairs for collision (broad phase collision)
    bool findCandidateTris(smMesh *mesh, smMesh  *mesh2);

    /// \brief find the candidate line-triangle pairs for collision (broad phase collision)
    bool findCandidateTrisLines(smMesh *mesh, smLineMesh *mesh2);

    /// \brief compute the collision between two triangles (narrow phase collision)
    void computeCollisionTri2Tri();

    /// \brief compute collision between a line segment and a triangle (narrow phase collision)
    void computeCollisionLine2Tri();

    /// \brief !!
    void filterLine2TrisResults();

    /// \brief !!
    void draw(const smDrawParam &param);

    /// \brief initialize the drawing structures
    void initDraw(const smDrawParam &param);

    /// \brief !!
    virtual void beginSim();

    /// \brief !!
    virtual void initCustom();

    /// \brief !! compute the hash
    void computeHash(smMesh *mesh, int *tris, int nbrTris);

    /// \brief !!
    virtual void run();

    /// \brief !!
    void endSim();

    /// \brief !! synchronize the buffers in the object..do not call by yourself.
    void syncBuffers();

};

#endif
