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

#ifndef SMLATTICE_H
#define SMLATTICE_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smCustomRenderer.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smSceneObject.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smGeometry.h"

#define SIMMEDTK_SPATIALGRID_MAXPRIMITIVES 500
#define SIMMEDTK_SPATIALGRID_MAXCELLS 1000
#define SIMMEDTK_SMLATTICE_NONE                 (0)
#define SIMMEDTK_SMLATTICE_ALL                  (1<<1)
#define SIMMEDTK_SMLATTICE_MINMAXPOINTS         (1<<2)
#define SIMMEDTK_SMLATTICE_SEPERATIONLINES      (1<<3)
#define SIMMEDTK_SMLATTICE_CELLS                    (1<<4)
#define SIMMEDTK_SMLATTICE_CELLCENTERS          (1<<5)
#define SIMMEDTK_SMLATTICE_CELLPOINTS               (1<<6)
#define SIMMEDTK_SMLATTICE_CELLPOINTSLINKS      (1<<7)
#define SIMMEDTK_SMLATTICE_CENTER                   (1<<8)
#define SIMMEDTK_SMLATTICE_CELLVERTICES         (1<<9)
#define SIMMEDTK_SMLATTICE_CELLACTIVEVERTICES       (1<<10)
#define SIMMEDTK_SMLATTICE_CELLTRIANGLES            (1<<11)

/// \brief !!
enum smLatticeReturnType
{
    SIMMEDTK_LATTICE_OK,
    SIMMEDTK_LATTICE_INVALIDPARAMS,
    SIMMEDTK_LATTICE_INVALIDBOUNDS
};

/// \brief !! holds the collision primitive pairs
struct smCollisionPairs
{
    smUnifiedID objectIndex;
    smUnifiedID objectIndex2;
    smInt primIndex;
    smInt primIndex2;
};

/// \brief cell primitive
struct smCellPrim
{
    smInt index;
    smInt objectId;
};

/// \brief contains everything related to a cell
class smCell
{

public:
    smInt id;
    smInt cellId[3];
    smVec3f cellCenter;
    smVec3f cellLeftCorner;
    smVec3f cellRightCorner;
    smCellPrim cellPrimitives[SIMMEDTK_SPATIALGRID_MAXPRIMITIVES];
    smInt lastPrimitiveIndex;
    smInt timeStamp;
    smBool isActive;

    smCell()
    {
    }
};

/// \brief !!
class smLattice: public smCoreClass
{

public:
    //these should be templated..Current design is based on the triangle
    smAABB *aabb;
    smSurfaceMesh *mesh;
    smCell *cells;
    smInt totalCells;
    smInt xSeperation;
    smInt ySeperation;
    smInt zSeperation;
    smFloat xStep;
    smFloat yStep;
    smFloat zStep;
    smVec3f latticeCenter;
    smInt time;
    smUnifiedID linkedObject;

    /// \brief !!
    void boundingBoxInit()
    {
        aabb = new smAABB[mesh->nbrTriangles];
    }

    /// \brief constructor
    smLattice();

    /// \brief get the size of the lattice cell side in x-direction
    smFloat getXStep();

    /// \brief get the size of the lattice cell side in y-direction
    smFloat getYStep();

    /// \brief get the size of the lattice cell side in z-direction
    smFloat getZStep();

    /// \brief get the center of the lattice
    smVec3f getLatticeCenter();

    /// \brief !! get the left corner of cell 0
    smVec3f getLeftMinCorner();

    /// \brief !! get the right corner of cell 0
    smVec3f getRightMaxCorner();

    /// \brief destructor
    ~smLattice();

    /// \brief Initialize the lattice
    smLatticeReturnType init(smVec3f p_leftCorner, smVec3f p_rightCorner,
                             smInt p_xSeperation, smInt p_ySeperation, smInt p_zSeperation);

    /// \brief !!
    void indexReset();

    /// \brief !!
    void isCellEmpty(smInt p_cellIndex);

    /// \brief !!
    virtual void  linkPrimitivetoCell(smInt p_primitiveIndex);

    /// \brief update the bounds of the lattice
    void updateBounds(smSurfaceMesh* p_mesh, smInt p_index);

    /// \brief update the bounds of the lattice
    void updateBounds();

    /// \brief !!
    void linkPrims();

    /// \brief !!
    void addObject(smSceneObject *obj);

    /// \brief render the lattice for visaulization
    void draw(smDrawParam p_params);

};

#endif
