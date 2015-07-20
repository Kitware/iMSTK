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
#include "Mesh.h"

// VEGA includes
#include "volumetricMesh.h"

/// \brief this class is derived from generic Mesh class. Tetrahedron are building blocks of this volume mesh.
///  It also retains the surface triangle structure for rendering and collision detection purposes.
///  This surface triangle structure might be extracted from the volume mesh while loading
class VolumeMesh: public Mesh
{
public:
    /// \brief constructor
    VolumeMesh();

    /// \brief constructor
    VolumeMesh(const MeshType &meshtype, std::shared_ptr<ErrorLog> log);

    /// \brief destructor
    ~VolumeMesh();

    /// \brief constructor
    void GenerateTetra(const std::string& fileName);

    /// \brief load tetrahedron
    bool LoadTetra(const std::string& fileName);

    /// \brief load surface triangles
    bool getSurface(const std::string& fileName);

    /// \brief read the boundary conditions
    bool readBC(const std::string& fileName);

    /// \brief initialize the surface mesh
    void initSurface();

    /// \brief copy the surface mesh
    void copySurface();

    /// \brief translate mesh vertices in x, y and z directions
    void translateVolumeMesh(const core::Vec3d &p_translate);

    /// \brief scale the volume mesh
    void scaleVolumeMesh(const core::Vec3d &p_scale);

    /// \brief rotate mesh vertices in x, y and z directions
    void rotVolumeMesh(const Matrix33d &p_rot);

    /// \brief load the mesh
    bool loadMesh(const std::string& fileName, const MeshFileType &fileType);

    /// \brief populate the mesh data from the vega volumetric mesh file format
    void importVolumeMeshFromVegaFormat(const std::shared_ptr<const VolumetricMesh> vega3dMesh, const bool preProcessingStage);

    /// \brief update the mesh data from the vega volumetric mesh after deformation happens
    void updateVolumeMeshFromVegaFormat(const std::shared_ptr<const VolumetricMesh> vega3dMesh);

public:
    /// push Mesh class specific errors here
    int nbrTetra; ///< number of tetrahedra
    int nbrNodes; ///< total number of nodes of the volume mesh
    std::shared_ptr<ErrorLog> log_VM; ///< log the errors with volume mesh class
    core::StdVector3d nodes; ///< data of nodal co-ordinates
    std::vector<Tetrahedra> tetra; ///< tetrahedra data
    std::vector<int> surfaceNodeIndex; ///<
    std::vector<bool> fixed; ///< indicates if the node is fixed or not
};

#endif
