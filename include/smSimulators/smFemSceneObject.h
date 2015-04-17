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

#ifndef SMFEMSCENEOBJECT_H
#define SMFEMSCENEOBJECT_H

// Eigen includes
#include <Eigen/LU>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smUtilities/smMatrix.h"
#include "smUtilities/smVector.h"

/// \brief Fem class
class smFemSceneObject: public smSceneObject
{
public:
    /// \brief constructor
    smFemSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);

    /// \brief build FEM LM matrix
    void buildLMmatrix();

    /// \brief compute FEM stiffness matrix
    void computeStiffness();

    /// \brief load a matrix from an external file
    smBool loadMatrix(const smString &fname, smMatrixf &a);

    /// \brief save a matrix to an external file
    smBool saveMatrix(const smString &fname, smMatrixf &a);

    /// \brief !!
    smFloat V(smInt xyz, smInt xyz123, smInt tet);

    /// \brief assemble FEM stiffness matrix
    void assembleK(smInt element, smMatrixf k);

    /// \brief compute the displacements for quasi-static simulation
    void calculateDisplacements_QStatic(smStdVector3f &vertices);

    /// \brief compute the displacements for dynamic simulation
    void calculateDisplacements_Dynamic(smStdVector3f &vertices);

    /// \brief !!
    void lumpMasses();

    /// \brief get the volume of tetrahedron given four sides
    smFloat tetraVolume(smVec3f &a, smVec3f &b, smVec3f &c, smVec3f &d);

    /// \brief !!
    virtual void serialize(void *p_memoryBlock);

    /// \brief !!
    virtual void unSerialize(void *p_memoryBlock);

    /// \brief render the FEM mesh
    virtual void draw(const smDrawParam &p_params);

    /// \brief !! This function does not clone!, it simply returns a pointer...this is dangerous
    virtual std::shared_ptr<smSceneObject> clone();

public:
    //fem objetc has two representations: surface and volume
    smVolumeMesh *v_mesh; ///< volume mesh
    smMatrixf LM; ///< LM matrix of FEM
    smMatrixf ID; ///< ID matrix of FEM
    smInt totalDof; ///< total degree of freedom of the fem mesh
    smFloat dT; ///< time step
    smFloat density; ///< density of material
    smBool dynamicFem; ///< true if the mesh is to be evolved in time
    smBool pullUp; ///< !!
    smMatrixf stiffnessMatrix; ///< FEM stiffness matrix
    smMatrixf Kinv; ///< inverse of FEM stiffness matrix
    smMatrix33f k33; ///< holds a 3x3 matrix used during FEM formulation
    smMatrix33f k33inv; ///< inverse of 3x3 matrix
    smVectorf displacements; ///< displacement vector of the FEM
    smVectorf temp_displacements; ///< temporary vector of displacements
    smVectorf displacements_prev; ///< displacement vector at previous time step
    smVectorf dymamic_temp; ///< !!
    smVectorf componentMasses; ///< !!
    smVectorf fm; ///< !!
    smVectorf fm_temp; ///< !!
    smVectorf totalDisp; ///< total displacement till a given time t
    smVectorf nodeMass; ///< nodal mass
    smVectorf viscosity; ///< viscosity
    smInt pulledNode; ///< ID of the node pulled by the user
};

#endif
