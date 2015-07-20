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

#ifndef SMSURFACEMESH_H
#define SMSURFACEMESH_H

// SimMedTK includes
#include "Mesh.h"

/// \brief this is the Surface Mesh class derived from generic Mesh class.
class smSurfaceMesh: public smMesh
{

public:
    /// \brief push smMesh class specific errors here
    std::shared_ptr<ErrorLog> log_SF;

    /// \brief constructor
    smSurfaceMesh();

    /// \brief constructor
    smSurfaceMesh(const smMeshType &meshtype, std::shared_ptr<ErrorLog> log);

    /// \brief destructor
    virtual ~smSurfaceMesh();

    /// \brief load the surface mesh
    bool loadMesh(const std::string& fileName, const smMeshFileType &fileType);

    /// \brief load the surface mesh using in-house code
    bool loadMeshLegacy(const std::string& fileName, const smMeshFileType &fileType);

    /// \brief load the surface mesh from 3ds format
    bool Load3dsMesh(const std::string& fileName);

    /// \brief load surface meshes using assimp library
    bool LoadMeshAssimp(const std::string& fileName);

    /// \brief print the details of the mesh
    void printPrimitiveDetails();
};

#endif
