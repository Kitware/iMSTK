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

#ifndef SMPHYSXVOLUMEMESH_H
#define SMPHYSXVOLUMEMESH_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"

/// \brief !!
struct smPhysXLink
{
    smInt tetraIndex;
    smFloat baryCetricDistance[4];
};

/// \brief neighbor tetrahedra of the node
struct smNeighborTetrasOfNode
{
    smInt nbrNeiTet;
    smInt *idx;
};

/// \brief PhysX volume mesh
class smPhysXVolumeMesh: public smVolumeMesh
{
public:
    smBool renderSurface; ///< true if surface is to be rendered
    smBool renderTetras; ///< true if tetrahedra is to be rendered
    std::vector<smPhysXLink> links; ///< !!
    smInt nbrLinks; ///< !! number of links
    smSurfaceMesh *surfaceMesh; ///< surface mesh
    std::vector<std::vector <smEdge>> tetraEdges; ///< edges of tetrahedra
    smNeighborTetrasOfNode *neiTet; ///< neighbor tetrahedra of a node
    smBool *drawTet; ///< true for those tetra to be rendered
    smBool *removedNode; ///< true for nodes to be removed

    /// \brief constructor
    smPhysXVolumeMesh();

    /// \brief destructor
    ~smPhysXVolumeMesh(void);

    /// \brief load the tetrahedra file in the PhysX format
    smBool loadTetFile(const smString& p_TetFileName, const smString& p_surfaceMesh);

    /// \brief !! update the surface vertices
    void updateSurfaceVertices();

    /// \brief draw PhysX volume mesh
    virtual void draw(smDrawParam p_params);

    /// \brief compute the edges of the tetrahedra
    void createEdgeofTetras();

    /// \brief find neighbor tetrahedra of node (brute force search)
    void findNeighborTetrasOfNode();
};

#endif
