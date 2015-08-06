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
    /// \brief calculates the normal of a triangle
    ///
    core::Vec3d computeTriangleNormal(int triangle)
    {
        auto t = this->triangleArray[triangle];

        const core::Vec3d &v0 = this->vertices[t[0]];

        return (this->vertices[t[1]]-v0).cross(this->vertices[t[2]]-v0).normalized();
    }

    ///
    /// \brief Calculate normals for all triangles
    ///
    void computeTriangleNormals()
    {
        triangleNormals.clear();
        for(const auto &t : this->triangleArray)
        {
            const core::Vec3d &v0 = this->vertices[t[0]];

            this->triangleNormals.push_back(this->vertices[t[1]]-v0).cross(this->vertices[t[2]]-v0).normalized();
        }
    }

    ///
    /// \brief Calculate normals for all triangles
    ///
    void computeVertexNormals()
    {
        for (size_t i = 0, end = this->vertices.size(); i < end; ++i)
        {
            this->vertexNormals.push_back(core::Vec3d::Zero());
            for(auto const &j : this->vertexTriangleNeighbors[i])
            {
                this->vertexNormals[i] += this->triangleNormals[j];
            }
            this->vertexNormals[i].normalize();
        }
    }

    ///
    /// \brief Calculate vertex neighbors
    ///
    void computeVertexNeighbors()
    {
        vertexNeighbors.resize(this->vertices.size());

        if(vertexTriangleNeighbors.size() == 0)
        {
            this->computeVertexTriangleNeighbors();
        }

        for (size_t i = 0, end = this->vertices.size(); i < end; ++i)
        {
            for (auto const &j : this->vertexTriangleNeighbors[i])
            {
                for(auto const &vertex : this->triangleArray[j])
                {
                    if(vertex != i)
                    {
                        this->vertexNeighbors[i].push_back(vertex);
                    }
                }
            }
        }
    }

    ///
    /// \brief initializes the vertex neighbors
    ///
    void computeVertexTriangleNeighbors()
    {
        vertexTriangleNeighbors.resize(this->vertices.size());

        int triangle = 0;
        for(auto const &t : this->triangleArray)
        {
            vertexTriangleNeighbors[t[0]].push_back(triangle);
            vertexTriangleNeighbors[t[1]].push_back(triangle);
            vertexTriangleNeighbors[t[2]].push_back(triangle);
            triangle++;
        }
    }


    ///
    /// \brief print the details of the mesh
    ///
    void print()
    {
        std::cout << "----------------------------\n";
        std::cout << "Mesh Info for   : " << this->getName() <<"\n\t";
        std::cout << "Num. vertices   : " << this->getNumberOfVertices() <<"\n\t";
        std::cout << "Num. triangles  : " << this->getTriangles().size() << "\n\t";
        std::cout << "Is mesh textured: " << this->isMeshTextured() << "\n";
        std::cout << "----------------------------\n";
    }

private:
    // SurfaceMesh class specific errors here
    std::shared_ptr<ErrorLog> logger;

    // List of triangle normals
    std::vector<core::Vec3d> triangleNormals;

    // List of vertex normals
    std::vector<core::Vec3d> vertexNormals;

    // List of triangle tangents
    std::vector<core::Vec3d> triangleTangents;

    // List of vertex tangents
    std::vector<core::Vec3d> vertexTangents;

    // List of vertex neighbors
    std::vector<std::vector<size_t>> vertexNeighbors;

    // List of vertex neighbors
    std::vector<std::vector<size_t>> vertexTriangleNeighbors;
};

#endif


