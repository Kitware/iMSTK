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

#include <mutex>

#define SIMMEDTK_SPATIALGRID_LEFTCORNER smVec3f(-10,-10,-10)
#define SIMMEDTK_SPATIALGRID_RIGHTCORNER  smVec3f(10,10,10)
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
    ~smSpatialGridWorker();

    /// \brief constructor
    smSpatialGridWorker();

    /// \brief constructor
    smSpatialGridWorker(smProcessID p_ID);

    /// \brief !!
    void checkNarrow(smLattice *p_latticeA, smLattice *p_latticeB, smInt p_primAIndex, smInt p_primBIndex);

    /// \brief !!
    void checkCells(smLattice *p_latticeA, smLattice *p_latticeB, smInt p_cellIndex);

    /// \brief collision workhorse function. This is where the collision happens
    virtual void kernel();

    /// \brief the collision is advanced here
    virtual void run();

    /// \brief rendering for collision visualization
    virtual void draw();

    /// \brief !!
    static void beginTriangles();

    /// \brief draws the triangle
    static void drawTriangle(smVec3f &p_1, smVec3f &p_2, smVec3f &p_3);

    /// \brief !!
    static void endTriangles();

};

/// \brief
class smSpatialGrid: public smCollisionDetection
{

private:
    smSpatialGridWorker *workerThreads; ///< !!
    smInt totalThreads; ///< number of total threads
    smSynchronization synch; ///< !!

    smVec3f leftCorner; ///< left corner of the grid
    smVec3f rightCorner; ///< right corner of the grid
    smInt xSeperation; ///< grid spacing in x-direction
    smInt ySeperation; ///< grid spacing in y-direction
    smInt zSeperation; ///< grid spacing in z-direction

    smLattice *latticeList[SIMMEDTK_SPATIALGRID_TOTALLATTICES]; ///<
    smInt totalLattices; ///< total number of lattices
    smBool listUpdated; ///< !!
    std::mutex listLock; ///< !!

    /// \brief !!
    void beginFrame();

    /// \brief !!
    void startWorkers();

protected:
    smInt maxPrims; ///< !! maximum primitives allowed

public:

    smPipe *pipe; ///< !!

    /// \brief constructor
    smSpatialGrid(smInt p_outOutputPrimSize = SIMMEDTK_SPATIALGRID_WORKER_COLLISIONPAIRS;


    /// \brief set the total number of threads for collision check
    void setTotalThreads(smInt p_totalThreads);

    /// \brief initialization
    void init();

    /// \brief run the collision checks in a loop here
    void run();

    /// \brief !!
    void updateList();

    /// \brief !!
    virtual void endFrame();

    /// \brief !!
    virtual void exec();

    /// \brief This function adds lattice and it is  thread safe.
    virtual smInt addLattice(smLattice *p_lat);

    /// \brief This function removes lattice and it is  thread safe.
    virtual void removeLattice(smLattice *p_lat, smInt p_listIndex);

    /// \brief !! renders the workers threads
    void draw(smDrawParam p_params);

};

#endif
