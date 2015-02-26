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

#ifndef SMSPATIALGRID_H
#define SMSPATIALGRID_H

#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"
#include "smCollision/smCollisionDetection.h"
#include "smRendering/smCustomRenderer.h"
#include "smCore/smScene.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smSynchronization.h"
#include "smCore/smWorkerThread.h"
#include "smMesh/smLattice.h"
#include "smMesh/smLatticeTypes.h"
#include "smCore/smSDK.h"
#include "smRendering/smGLRenderer.h"
#include "smCore/smGeometry.h"

#include <QThread>
#include <mutex>

#define SIMMEDTK_SPATIALGRID_LEFTCORNER smVec3<smFloat>(-10,-10,-10)
#define SIMMEDTK_SPATIALGRID_RIGHTCORNER  smVec3<smFloat>(10,10,10)
#define SIMMEDTK_SPATIALGRID_XSEPERATION 10
#define SIMMEDTK_SPATIALGRID_YSEPERATION 10
#define SIMMEDTK_SPATIALGRID_ZSEPERATION 10
#define SIMMEDTK_SPATIALGRID_WORKER_COLLISIONPAIRS  1000
#define SIMMEDTK_SPATIALGRID_TOTALLATTICES  500

/// \brief !!
class smSpatialGridWorker: public smWorkerThread
{

    smLattice *latticePair; ///< !!
    smLattice *latticePair2; ///< !!

public:
    smCollidedTriangles *pairs; ///< collided triangle pairs
    smInt collidedPairs; ///< !!

    ///this is public for now
    smLattice *latticeList[SIMMEDTK_SPATIALGRID_TOTALLATTICES]; ///<
    smInt totalLattices; ///< total number of lattices

    /// \brief destructor
    ~smSpatialGridWorker()
    {
    }

    /// \brief constructor
    smSpatialGridWorker()
    {
        totalLattices = 0;
        collidedPairs = 0;
    }

    /// \brief constructor
    smSpatialGridWorker(smProcessID p_ID): smWorkerThread(p_ID)
    {
        totalLattices = 0;
    }

    /// \brief !!
    inline void checkNarrow(smLattice *p_latticeA, smLattice *p_latticeB, smInt p_primAIndex, smInt p_primBIndex)
    {

        smInt coPlanar;
        smVec3<smFloat> interSectPoint1;
        smVec3<smFloat> interSectPoint2;
        smVec3<smFloat> projectedPoint1;
        smVec3<smFloat> projectedPoint2;
        smShort singlePoint1;
        smShort singlePoint2;

        if (smCollisionUtils::tri2tri(p_latticeA->mesh->vertices[p_latticeA->mesh->triangles[p_primAIndex].vert[0]],
                                      p_latticeA->mesh->vertices[p_latticeA->mesh->triangles[p_primAIndex].vert[1]],
                                      p_latticeA->mesh->vertices[p_latticeA->mesh->triangles[p_primAIndex].vert[2]],
                                      p_latticeB->mesh->vertices[p_latticeB->mesh->triangles[p_primBIndex].vert[0]],
                                      p_latticeB->mesh->vertices[p_latticeB->mesh->triangles[p_primBIndex].vert[1]],
                                      p_latticeB->mesh->vertices[p_latticeB->mesh->triangles[p_primBIndex].vert[2]],
                                      coPlanar,
                                      interSectPoint1,
                                      interSectPoint2,
                                      singlePoint1,
                                      singlePoint2,
                                      projectedPoint1,
                                      projectedPoint2))
        {

            pairs[collidedPairs].point1 = singlePoint1;
            pairs[collidedPairs].point2 = singlePoint2;

            pairs[collidedPairs].proj1 = projectedPoint1;
            pairs[collidedPairs].proj2 = projectedPoint2;

            pairs[collidedPairs].tri1.meshID = p_latticeA->mesh->uniqueId;
            pairs[collidedPairs].tri2.meshID = p_latticeB->mesh->uniqueId;

            pairs[collidedPairs].tri1.primID = p_primAIndex;
            pairs[collidedPairs].tri2.primID = p_primBIndex;
            pairs[collidedPairs].tri1.vert[0] = p_latticeA->mesh->vertices [p_latticeA->mesh->triangles[p_primAIndex].vert[0]];
            pairs[collidedPairs].tri1.vert[1] = p_latticeA->mesh->vertices [p_latticeA->mesh->triangles[p_primAIndex].vert[1]];
            pairs[collidedPairs].tri1.vert[2] = p_latticeA->mesh->vertices [p_latticeA->mesh->triangles[p_primAIndex].vert[2]];

            pairs[collidedPairs].tri2.vert[0] = p_latticeB->mesh->vertices [p_latticeB->mesh->triangles[p_primBIndex].vert[0]];
            pairs[collidedPairs].tri2.vert[1] = p_latticeB->mesh->vertices [p_latticeB->mesh->triangles[p_primBIndex].vert[1]];
            pairs[collidedPairs].tri2.vert[2] = p_latticeB->mesh->vertices [p_latticeB->mesh->triangles[p_primBIndex].vert[2]];

            collidedPairs++;
        }
    }

    /// \brief !!
    inline void checkCells(smLattice *p_latticeA, smLattice *p_latticeB, smInt p_cellIndex)
    {

        for (smInt i = 0; i < p_latticeA->cells[p_cellIndex].lastPrimitiveIndex; i++)
            for (smInt j = 0; j < p_latticeB->cells[p_cellIndex].lastPrimitiveIndex; j++)
            {
                if (!smAABB::checkOverlap(p_latticeA->aabb[p_latticeA->cells[p_cellIndex].cellPrimitives[i].index],
                                          p_latticeB->aabb[p_latticeB->cells[p_cellIndex].cellPrimitives[j].index]))
                {
                    continue;
                }

                checkNarrow(p_latticeA, p_latticeB, p_latticeA->cells[p_cellIndex].cellPrimitives[i].index, p_latticeB->cells[p_cellIndex].cellPrimitives[j].index);
            }
    }

    /// \brief collision workhorse function. This is where the collision happens
    virtual void kernel()
    {

        collidedPairs = 0;

        for (smInt i = 0; i < totalLattices; i++)
        {
            latticeList[i]->indexReset();
            latticeList[i]->updateBounds();
            latticeList[i]->linkPrims();
        }

        for (smInt i = 0; i < totalLattices; i++)
        {
            latticePair = latticeList[i];

            for (smInt j = i + 1; j < totalLattices; j++)
            {
                latticePair2 = latticeList[j];

                for (smInt cellIndex = 0; cellIndex < latticePair->totalCells; cellIndex++)
                {

                    //no primitives inside the cell
                    if (latticePair2->cells[cellIndex].lastPrimitiveIndex == 0 || latticePair->cells[cellIndex].lastPrimitiveIndex == 0)
                    {
                        continue;
                    }

                    checkCells(latticePair, latticePair2, cellIndex);

                }
            }
        }
    }

    /// \brief the collision is advanced here
    virtual void run()
    {
        while (true || !termination)
        {
            synch->waitTaskStart();
            kernel();
            synch->signalTaskDone();
        }
    }

    /// \brief rendering for collision visualization
    virtual void draw()
    {
        smInt v[3];
        smBaseMesh *baseMesh;
        smMesh *mesh;

        smGLRenderer::beginTriangles();
        glColor3fv((GLfloat*)&smColor::colorRed);

        for (smInt i = 0; i < collidedPairs; i++)
        {
            baseMesh = smSDK::getMesh(pairs[i].tri1.meshID); //correct that later on

            if (baseMesh->getType() == SIMMEDTK_SMMESH)
            {
                mesh = (smMesh*)baseMesh;
            }

            v[0] = mesh->triangles[pairs[i].tri1.primID].vert[0];
            v[1] = mesh->triangles[pairs[i].tri1.primID].vert[1];
            v[2] = mesh->triangles[pairs[i].tri1.primID].vert[2];

            smGLRenderer::drawTriangle(mesh->vertices[v[0]], mesh->vertices[v[1]], mesh->vertices[v[2]]);
        }

        smGLRenderer::endTriangles();
        glColor3fv((GLfloat*)&smColor::colorWhite);
    }

    /// \brief !!
    inline static void beginTriangles()
    {
        glBegin(GL_TRIANGLES);
    }

    /// \brief draws the triangle
    inline static void drawTriangle(smVec3<smFloat> &p_1, smVec3<smFloat> &p_2, smVec3<smFloat> &p_3)
    {
        glVertex3fv((GLfloat*)&p_1);
        glVertex3fv((GLfloat*)&p_2);
        glVertex3fv((GLfloat*)&p_3);
    }

    /// \brief !!
    inline static void endTriangles()
    {
        glEnd();
    }

};

/// \brief
class smSpatialGrid: public smCollisionDetection, QThread
{

private:
    smSpatialGridWorker *workerThreads; ///< !!
    smInt totalThreads; ///< number of total threads
    smSynchronization synch; ///< !!

    smVec3<smFloat> leftCorner; ///< left corner of the grid
    smVec3<smFloat> rightCorner; ///< right corner of the grid
    smInt xSeperation; ///< grid spacing in x-direction
    smInt ySeperation; ///< grid spacing in y-direction
    smInt zSeperation; ///< grid spacing in z-direction

    smLattice *latticeList[SIMMEDTK_SPATIALGRID_TOTALLATTICES]; ///<
    smInt totalLattices; ///< total number of lattices
    smBool listUpdated; ///< !!
    std::mutex listLock; ///< !!

    /// \brief !!
    void beginFrame()
    {
    }

    /// \brief !!
    void startWorkers()
    {
        for (smInt i = 0; i < totalThreads; i++)
        {
            workerThreads[i].start();
        }
    }

protected:
    smInt maxPrims; ///< !! maximum primitives allowed

public:

    smPipe *pipe; ///< !!

    /// \brief constructor
    smSpatialGrid(smInt p_outOutputPrimSize = SIMMEDTK_SPATIALGRID_WORKER_COLLISIONPAIRS): synch(1)
    {

        maxPrims = p_outOutputPrimSize;
        totalThreads = 1;
        leftCorner = SIMMEDTK_SPATIALGRID_LEFTCORNER;
        rightCorner = SIMMEDTK_SPATIALGRID_RIGHTCORNER;
        xSeperation = SIMMEDTK_SPATIALGRID_XSEPERATION;
        ySeperation = SIMMEDTK_SPATIALGRID_YSEPERATION;
        zSeperation = SIMMEDTK_SPATIALGRID_ZSEPERATION;
        totalLattices = 0;

        for (smInt i = 0; i < SIMMEDTK_SPATIALGRID_TOTALLATTICES; i++)
        {
            latticeList[i] = NULL;
        }

        listUpdated = false;
        pipe = new smPipe("col_grid_tri2tri", sizeof(smCollidedTriangles), maxPrims);
    }


    /// \brief set the total number of threads for collision check
    void setTotalThreads(smInt p_totalThreads)
    {

        if (isInitialized != false)
        {
            totalThreads = p_totalThreads;
        }

        synch.setWorkerCounter(p_totalThreads);

    }

    /// \brief initialization
    void init()
    {
        smProcessID id;

        id.numbScheme = SIMMEDTK_PROCNUMSCHEME_X__;

        if (isInitialized == true)
        {
            return;
        }

        workerThreads = new(smSpatialGridWorker[totalThreads]);

        for (smInt i = 0; i < totalThreads; i++)
        {
            id.x = i;
            id.totalProcX = totalThreads;
            workerThreads[i].setSynchObject(synch);
            workerThreads[i].setId(id);
        }

        updateList();
        isInitialized = true;
    }

    /// \brief run the collision checks in a loop here
    void run()
    {
        smCollidedTriangles *tristrisHead, *tristrisCurrent;
        smInt nbrTriTriCollisions = 0;

        if (isInitialized == false)
        {
            return;
        }

        startWorkers();

        while (true && this->terminateExecution == false)
        {
            beginModule();
            nbrTriTriCollisions = 0;
            synch.startTasks();
            tristrisHead = (smCollidedTriangles*)pipe->beginWrite();
            tristrisCurrent = tristrisHead;

            for (smInt i = 0; i < totalThreads; i++)
            {
                if ((workerThreads[i].collidedPairs + nbrTriTriCollisions) < maxPrims)
                {
                    memcpy(tristrisCurrent, workerThreads->pairs, sizeof(smCollidedTriangles)*workerThreads[i].collidedPairs);
                    tristrisCurrent += workerThreads[i].collidedPairs;
                    nbrTriTriCollisions += workerThreads[i].collidedPairs;
                }
            }

            pipe->endWrite(nbrTriTriCollisions);
            pipe->acknowledgeValueListeners();
            endModule();
        }

        terminationCompleted = true;
    }

    /// \brief !!
    void updateList()
    {

        smLattice **tempLatticeList = new(smLattice*[SIMMEDTK_SPATIALGRID_TOTALLATTICES]);
        smInt index = 0;
        std::lock_guard<std::mutex> lock(listLock); //Lock is released when leaves scope

        if (listUpdated == true)
        {
            for (smInt i = 0; i < SIMMEDTK_SPATIALGRID_TOTALLATTICES; i++)
            {
                if (latticeList[i] != NULL)
                {
                    tempLatticeList[index] = latticeList[i];
                    index++;
                }
            }

            memcpy(&latticeList[0], &tempLatticeList[0], sizeof(smLattice*)*SIMMEDTK_SPATIALGRID_TOTALLATTICES);
            totalLattices = index;

            for (smInt i = 0; i < totalThreads; i++)
            {
                memcpy(&workerThreads[i].latticeList[0], tempLatticeList, sizeof(smLattice*)*SIMMEDTK_SPATIALGRID_TOTALLATTICES);
                workerThreads[i].totalLattices = totalLattices;
            }
        }

        listUpdated = false;
        delete []tempLatticeList;
    }

    /// \brief !!
    virtual void endFrame()
    {
        updateList();
    }

    /// \brief !!
    virtual void exec()
    {
        if (isInitialized)
        {
            start();
        }
        else
        {
            init();
            start();
        }
    }

    /// \brief This function adds lattice and it is  thread safe.
    virtual smInt addLattice(smLattice *p_lat)
    {

        smSDK::addRef(p_lat);
        std::lock_guard<std::mutex> lock(listLock); //Lock is released when leaves scope
        latticeList[totalLattices] = p_lat;
        listUpdated = true;
        p_lat->init(SIMMEDTK_SPATIALGRID_LEFTCORNER, SIMMEDTK_SPATIALGRID_RIGHTCORNER,
                    SIMMEDTK_SPATIALGRID_XSEPERATION,
                    SIMMEDTK_SPATIALGRID_YSEPERATION,
                    SIMMEDTK_SPATIALGRID_ZSEPERATION);
        totalLattices++;
        return totalLattices - 1;
    }

    /// \brief This function removes lattice and it is  thread safe.
    virtual void removeLattice(smLattice *p_lat, smInt p_listIndex)
    {
        smSDK::removeRef(p_lat);
        std::lock_guard<std::mutex> lock(listLock); //Lock is released when leaves scope
        latticeList[p_listIndex] = NULL;
        totalLattices--;
        listUpdated = true;
    }

    /// \brief !! renders the workers threads
    void draw(smDrawParam p_params)
    {
        for (smInt i = 0; i < totalThreads; i++)
        {
            workerThreads[i].draw();
        }
    }

};

#endif
