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
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Rendering/CustomRenderer.h"
#include "SurfaceMesh.h"
#include "Core/SceneObject.h"
#include "Core/StaticSceneObject.h"
#include "Core/Geometry.h"

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
    std::shared_ptr<UnifiedId> objectIndex;
    std::shared_ptr<UnifiedId> objectIndex2;
    int primIndex;
    int primIndex2;
};

/// \brief cell primitive
struct smCellPrim
{
    int index;
    int objectId;
};

/// \brief contains everything related to a cell
class smCell
{

public:
    int id;
    int cellId[3];
    core::Vec3d cellCenter;
    core::Vec3d cellLeftCorner;
    core::Vec3d cellRightCorner;
    smCellPrim cellPrimitives[SIMMEDTK_SPATIALGRID_MAXPRIMITIVES];
    int lastPrimitiveIndex;
    int timeStamp;
    bool isActive;

    smCell()
    {
    }
};

/// \brief !!
class smLattice: public CoreClass
{
public:
    /// \brief !!
    void boundingBoxInit()
    {
        aabb = new AABB[mesh->nbrTriangles];
    }

    /// \brief constructor
    smLattice();

    /// \brief get the size of the lattice cell side in x-direction
    float getXStep();

    /// \brief get the size of the lattice cell side in y-direction
    float getYStep();

    /// \brief get the size of the lattice cell side in z-direction
    float getZStep();

    /// \brief get the center of the lattice
    core::Vec3d getLatticeCenter();

    /// \brief !! get the left corner of cell 0
    core::Vec3d getLeftMinCorner();

    /// \brief !! get the right corner of cell 0
    core::Vec3d getRightMaxCorner();

    /// \brief destructor
    ~smLattice();

    /// \brief Initialize the lattice
    smLatticeReturnType init(core::Vec3d p_leftCorner, core::Vec3d p_rightCorner,
                             int p_xSeperation, int p_ySeperation, int p_zSeperation);

    /// \brief !!
    void indexReset();

    /// \brief !!
    void isCellEmpty(int p_cellIndex);

    /// \brief !!
    virtual void  linkPrimitivetoCell(int p_primitiveIndex);

    /// \brief update the bounds of the lattice
    void updateBounds(std::shared_ptr<smSurfaceMesh> p_mesh, int p_index);

    /// \brief update the bounds of the lattice
    void updateBounds();

    /// \brief !!
    void linkPrims();

    /// \brief !!
    void addObject(SceneObject *obj);

public:
    friend class smLatticeRenderDelegate;

    //these should be templated..Current design is based on the triangle
    AABB *aabb;
    std::shared_ptr<smSurfaceMesh> mesh;
    smCell *cells;
    int totalCells;
    int xSeperation;
    int ySeperation;
    int zSeperation;
    float xStep;
    float yStep;
    float zStep;
    core::Vec3d latticeCenter;
    int time;
    std::shared_ptr<UnifiedId> linkedObject;
};

#endif
