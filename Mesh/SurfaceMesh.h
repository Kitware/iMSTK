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
    ~SurfaceMesh();

    ///
    /// \brief calculates the normal of a triangle
    ///
    core::Vec3d computeTriangleNormal(int triangle);

    ///
    /// \brief Calculate normals for all triangles
    ///
    void computeTriangleNormals();

    ///
    /// \brief Calculate normals for all triangles
    ///
    void computeVertexNormals();

    ///
    /// \brief Calculate vertex neighbors
    ///
    void computeVertexNeighbors();

    ///
    /// \brief initializes the vertex neighbors
    ///
    void computeVertexTriangleNeighbors();

    ///
    /// \brief Compute tangents for triangles
    ///
    void computeTriangleTangents();

    ///
    /// \brief Check for the correct triangle orientation.
    ///
    void checkTriangleOrientation();

    /// Texture handling

    struct TextureAttachment;
    ///
    /// \brief Returns the texture coordinates array.
    ///
    const std::vector<core::Vec2f,
    Eigen::aligned_allocator<core::Vec2f>> &getTextureCoordinates() const;
    std::vector<core::Vec2f,
    Eigen::aligned_allocator<core::Vec2f>> &getTextureCoordinates();

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    const std::vector<std::shared_ptr<TextureAttachment>> &getTextures() const;

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    const int &getTextureId(size_t i) const;

    ///
    /// \brief Return true if it contains texture coordinates
    ///
    bool hasTextureCoordinates() const;

    ///
    /// \brief Add texture coordinates
    ///
    void addTextureCoordinate(const core::Vec2f &coord);
    void addTextureCoordinate(const float &x, const float &y);

    ///
    /// \brief Assign the texture
    ///
    void assignTexture(const std::string& referenceName);

    ///
    /// \brief Query if the mesh has textures available for rendering
    ///
    bool isMeshTextured() const;

    ///
    /// \brief Set/get use OBJ of ThreDS textures coordinates
    ///
    void setUseOBJTexture(bool use);
    void setUseThreDSTexture(bool use);

    ///
    /// \brief Get triangle normals
    ///
    const core::Vec3d &getTriangleNormal(size_t i) const;

    ///
    /// \brief Get tangents
    ///
    const core::Vec3d &getTriangleTangent(size_t i) const;

    ///
    /// \brief Get vertex normal
    ///
    const core::Vec3d &getVertexNormal(size_t i) const;

    ///
    /// \brief Get vertex normal
    ///
    const std::vector<core::Vec3d> &getVertexNormals() const
    {
        return this->vertexNormals;
    }
    std::vector<core::Vec3d> &getVertexNormals()
    {
        return this->vertexNormals;
    }

    ///
    /// \brief Get vertex tangents
    ///
    const core::Vec3d &getVertexTangent(size_t i) const;
    const std::vector<core::Vec3d> &getVertexTangents() const;
    std::vector<core::Vec3d> &getVertexTangents();

    ///
    /// \brief print the details of the mesh
    ///
    void print() const override;

    ///
    /// \brief Get the total number of triangles
    ///
    size_t getNumberOfTriangles() const;


private:
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

    // Textures attached to this mesh.
    std::vector<std::shared_ptr<TextureAttachment>> textures;

    // Texture coordinates
    std::vector<core::Vec2f,
    Eigen::aligned_allocator<core::Vec2f>> textureCoord;

    bool useThreeDSTexureCoordinates;
    bool useOBJDSTexureCoordinates;
};

///
/// \brief Texture management structure
///
struct SurfaceMesh::TextureAttachment
{
    TextureAttachment() :
    textureId(-1),
    textureName("")
    {
    }

    // Texture id
    int textureId;

    // Texture internal name
    std::string textureName;
};

#endif



