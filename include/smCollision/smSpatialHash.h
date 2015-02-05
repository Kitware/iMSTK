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

#ifndef SMSPATIALHASH_H
#define SMSPATIALHASH_H

#include "smCore/smConfig.h"
#include "smCollision/smCollisionConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smDataStructs.h"
#include "smCore/smPipe.h"

class smPipe;
template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
struct smOctreeCell;
template<typename smSurfaceTreeCell> struct smSurfaceTreeIterator;
typedef smHash<smCellPoint> smHashCellPoint;

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

inline unsigned int HASH(unsigned int p_SIZE, unsigned int p_x, unsigned int p_y, unsigned int p_z)
{
    return (((((p_x) * HASH_P1) ^ ((p_y) * HASH_P2) ^ ((p_z) * HASH_P3))) % (p_SIZE));
}

/// \brief spatial hash
class smSpatialHash: public smObjectSimulator
{

protected:
    smFloat cellSizeX; ///< cell size in x-direction
    smFloat cellSizeY; ///< cell size in y-direction
    smFloat cellSizeZ; ///< cell size in z-direction

    //These structures below are Triangle2Triangle collisions

    /// \brief Cells for triangles. It stores candidate triangles
    smHash<smCellTriangle> cells;

    /// \brief structure that stores the meshes in the scene
    QVector<smMesh *> meshes;

    /// \brief After the collision is cimpleted the result is written in here
    smCollidedTriangles *collidedPrims;

    /// \brief Number of collisions that triangles are stored.
    smInt nbrTriCollisions;

    /// \brief Line mesh structure that is added to collision detection engine
    QVector<smLineMesh *> lineMeshes;

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
    smInt nbrLineTriCollisions;

    /// \brief smHash<smCollisionGroup> collisionGroups;

    /// \brief the collision results are here
    smCollidedModelPoints *collidedModelPoints;

    /// \brief the number of collisions for model to point
    smInt nbrModelPointCollisions;

    /// \brief For maximum collision output.
    smInt maxPrims;

    map<smInt, smInt> filteredList;

    /// \brief adds triangle to hash
    inline void addTriangle(smMesh *p_mesh, smInt p_triangleId, smHash<smCellTriangle> &p_cells);

    /// \brief adds line to hash
    inline void addLine(smLineMesh *p_mesh, smInt p_edgeId, smHash<smCellLine> &p_cells);

    /// \brief adds point to hash
    inline void addPoint(smMesh *p_mesh, smInt p_vertId, smHash<smCellPoint> p_cells);

    /// \brief adds octree cell to hash
    inline void addOctreeCell(smSurfaceTree<smOctreeCell> *p_colModel, smHash<smCellModel> p_cells);

    /// \brief !!
    void reset();

    vector<smSurfaceTree<smOctreeCell>*> colModel;

public:
    smBool enableDuplicateFilter; ///< !!
    smPipe *pipe; ///< !!
    smPipe *pipeTriangles; ///< !!
    smPipe *pipeModelPoints; ///< !!

    /// \brief !!
    void  findCandidatePoints(smMesh *p_mesh, smSurfaceTree<smOctreeCell> *p_colModel);

    /// \brief !!
    void computeCollisionModel2Points();

    /// \brief destructor clear some memory
    ~smSpatialHash();

    /// \brief !!
    void addCollisionModel(smSurfaceTree<smOctreeCell> *p_CollMode);

    /// \brief !!
    void addMesh(smMesh *mesh);

    /// \brief !!
    void addMesh(smLineMesh *p_mesh);

    /// \brief !!
    void removeMesh(smMesh *p_mesh);

    /// \brief !!
    smSpatialHash(smErrorLog *p_errorLog,
                  smInt p_hashTableSize,
                  smFloat p_cellSizeX,
                  smFloat p_cellSizeY,
                  smFloat p_cellSizeZ,
                  smInt p_outOutputPrimSize = SIMMEDTK_COLLISIONOUTPUTBUF_SIZE);

    /// \brief !!
    void draw();

    /// \brief find the candidate triangle pairs for collision (broad phase collision)
    smBool findCandidateTris(smMesh *p_mesh, smMesh  *p_mesh2);

    /// \brief find the candidate line-triangle pairs for collision (broad phase collision)
    smBool findCandidateTrisLines(smMesh *p_mesh, smLineMesh *p_mesh2);

    /// \brief compute the collision between two triangles (narrow phase collision)
    void computeCollisionTri2Tri();

    /// \brief compute collision between a line segment and a triangle (narrow phase collision)
    void computeCollisionLine2Tri();

    /// \brief !!
    void filterLine2TrisResults();

    /// \brief !!
    void draw(smDrawParam p_param);

    /// \brief initialize the drawing structures
    void initDraw(smDrawParam p_param);

    /// \brief !!
    virtual void beginSim()
    {

        smObjectSimulator::beginSim();
        //start the job
        nbrTriCollisions = 0;
        nbrLineTriCollisions = 0;
        nbrModelPointCollisions = 0;

        for (smInt i = 0; i < meshes.size(); i++)
        {
            meshes[i]->updateTriangleAABB();
        }

        for (smInt i = 0; i < lineMeshes.size(); i++)
        {
            meshes[i]->upadateAABB();
        }
    }

    /// \brief !!
    virtual void initCustom();

    /// \brief !! compute the hash
    inline void computeHash(smMesh *p_mesh, int *p_tris, int p_nbrTris);

    /// \brief !!
    virtual void run()
    {

        smTimer timer;
        timer.start();
        beginSim();

        for (smInt i = 0; i < colModel.size(); i++)
            for (smInt i = 0; i < meshes.size(); i++)
            {
                findCandidatePoints(meshes[i], colModel[i]);
                addOctreeCell(colModel[i], cellsForModel);
            }

        ///Triangle-Triangle collision
        for (smInt i = 0; i < meshes.size(); i++)
        {
            for (smInt j = i + 1; j < meshes.size(); j++)
            {
                if (meshes[i]->collisionGroup.isCollisionPermitted(meshes[j]->collisionGroup))
                {
                    if (findCandidateTris(meshes[i], meshes[j]) == false)
                    {
                        continue;
                    }
                }
            }
        }

        ///Triangle-line Collision
        for (smInt i = 0; i < meshes.size(); i++)
            for (smInt j = 0; j < lineMeshes.size(); j++)
            {
                if (meshes[i]->collisionGroup.isCollisionPermitted(lineMeshes[j]->collisionGroup))
                {
                    if (findCandidateTrisLines(meshes[i], lineMeshes[j]) == false)
                    {
                        continue;
                    }
                }
            }

        computeCollisionTri2Tri();
        computeCollisionLine2Tri();
        computeCollisionModel2Points();
        endSim();
    }

    /// \brief !!
    void endSim()
    {
        //end the job
        smObjectSimulator::endSim();
        reset();
    }

    /// \brief !! synchronize the buffers in the object..do not call by yourself.
    void syncBuffers()
    {
    }

};

#endif
