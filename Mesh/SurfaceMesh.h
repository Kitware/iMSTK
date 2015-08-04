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
#include "Core/BaseMesh.h"

/// \brief this is the Surface Mesh class derived from generic Mesh class.
class SurfaceMesh: public Core::BaseMesh
{

public:
    ///
    /// \brief constructor
    ///
    SurfaceMesh();

    ///
    /// \brief destructor
    ///
    virtual ~SurfaceMesh();

    ///
    /// \brief load the surface mesh
    ///
    bool load(const std::string& fileName);

    ///
    /// \brief print the details of the mesh
    ///
    void print();

private:
    // SurfaceMesh class specific errors here
    std::shared_ptr<ErrorLog> logger;

    // List of triangles
    std::vector<std::array<int,3>> triangles;

    // List of triangle normals
    std::vector<core::Vec3d> triangleNormals;

    // List of vertex normals
    std::vector<core::Vec3d> vertexNormals;

    // List of triangle tangents
    std::vector<core::Vec3d> triangleTangents;

    // List of vertex tangents
    std::vector<core::Vec3d> vertexTangents;
};

#endif
