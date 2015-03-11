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

#ifndef SMVOLUMEMESH_H
#define SMVOLUMEMESH_H

// SimMedTK includes
#include "smMesh/smMesh.h"

/// \brief this class is derived from generic Mesh class. Tetrahedron are building blocks of this volume mesh.
///  It also retains the surface triangle structure for rendering and collision detection purposes.
///  This surface triangle structure might be extracted from the volume mesh while loading
class smVolumeMesh: public smMesh
{

public:
    /// push smMesh class specific errors here
    smErrorLog *log_VM; ///< log the errors with volume mesh class
    smInt nbrNodes; ///< total number of nodes of the volume mesh
    smVec3f *nodes; ///< data of nodal co-ordinates
    smInt nbrTetra; ///< number of tetrahedra
    smTetrahedra *tetra; ///< tetrahedra data
    smInt *surfaceNodeIndex; ///<
    smBool *fixed; ///< indicates if the node is fixed or not

    /// \brief constructor
    smVolumeMesh()
    {
        nbrNodes = 0;
        nbrTetra = 0;
        tetra = NULL;
        surfaceNodeIndex = NULL;
        fixed = NULL;
    }

    /// \brief constructor
    smVolumeMesh(smMeshType meshtype, smErrorLog *log);

    /// \brief destructor
    ~smVolumeMesh();

    /// \brief constructor
    void GenerateTetra(smChar *fileName);

    /// \brief load tetrahedron
    smBool LoadTetra(const smChar *fileName);

    /// \brief load surface triangles
    smBool getSurface(const smChar *fileName);

    /// \brief read the boundary conditions
    smBool readBC(const smChar *fileName);

    /// \brief initialize the surface mesh
    void initSurface();

    /// \brief copy the surface mesh
    void copySurface();

    /// \brief translate mesh vertices in x, y and z directions
    void translateVolumeMesh(smVec3f p_translate);

    /// \brief scale the volume mesh
    void scaleVolumeMesh(smVec3f p_scale);

    /// \brief rotate mesh vertices in x, y and z directions
    void rotVolumeMesh(smMatrix33f p_rot);

    /// \brief load the mesh
    smBool loadMesh(smChar *fileName, smMeshFileType fileType);
};

#endif
