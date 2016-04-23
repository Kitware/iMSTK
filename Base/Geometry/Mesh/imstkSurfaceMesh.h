/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#ifndef imstkSurfaceMesh_h
#define imstkSurfaceMesh_h

#include <array>
#include <set>

#include "imstkMesh.h"

namespace imstk {

///
/// \class SurfaceMesh
///
/// \brief Surface triangular mesh
///
class SurfaceMesh : public Mesh
{
public:

    using TriangleArray = std::array<size_t, 3>;
    using NeighborsType = std::set<size_t>;

    SurfaceMesh() : Mesh(GeometryType::SurfaceMesh) {}

    ~SurfaceMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures gives vertex positions and
    ///     triangle connectivity
    ///
    void initialize(const std::vector<Vec3d>& vertices,
                    const std::vector<TriangleArray>& triangles,
                    const std::vector<Vec2f>& texCoords = std::vector<Vec2f>(),
                    const bool computDerivedData = false);

    ///
    /// \brief Computes neighboring triangles for all vertices
    ///
    void computeVerticesNeighborTriangles();

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    void computeVerticesNeighborVertices();

    ///
    /// \brief Compute the normals to the triangles
    ///
    void computeTrianglesNormals();

    ///
    /// \brief Computes the normals of all the vertices
    ///
    void computeVerticesNormals();

    ///
    /// \brief Computes the tangents
    ///
    void computeVerticesTangents();

    ///
    /// \brief Get the volume enclosed by the surface mesh
    ///
    double getVolume() const;

    ///
    /// \brief Clear all the mesh data
    ///
    void clear();

    ///
    /// \brief Print the mesh
    ///
    void print() const;

    // Accessors

    ///
    /// \brief Get/Set triangle connectivity
    ///
    void setTrianglesVertices(const std::vector<TriangleArray>& triangles);
    const std::vector<TriangleArray>& getTrianglesVertices() const;

    ///
    /// \brief Get/Set texture coordinates
    ///
    void setTextureCoordinates(const std::vector<Vec2f>& coords);
    const std::vector<Vec2f>& getTextureCoordinates() const;

    ///
    /// \brief Get vector of normals of all the triangles
    ///
    const std::vector<Vec3d>& getTrianglesNormals() const;

    ///
    /// \brief Get normal of a triangle given its index
    ///
    const Vec3d& getTriangleNormal(size_t i) const;

    ///
    /// \brief Get vector of normals of all the vertices
    ///
    const std::vector<Vec3d>& getVerticesNormals() const;

    ///
    /// \brief Get normal of a vertex given its index
    ///
    const Vec3d& getVerticeNormal(size_t i) const;

    ///
    /// \brief Get/Set vertex tangents
    ///
    const std::vector<Vec4d>& getVerticesTangents() const;
    const Vec4d& getVerticeTangent(size_t i) const;

    ///
    /// \brief Returns the number of triangles
    ///
    int getNumTriangles() const;

protected:

    std::vector<TriangleArray> m_trianglesVertices; ///> Triangle connectivity
    std::vector<Vec2f> m_textureCoordinates; ///> Texture coordinates

    std::vector<NeighborsType> m_verticesNeighborTriangles; ///> Neighbor triangles to vertices
    std::vector<NeighborsType> m_verticesNeighborVertices; ///> Neighbor vertices to vertices

    std::vector<Vec3d> m_trianglesNormals; ///> Normals to the triangles
    std::vector<Vec3d> m_verticesNormals; ///> Normals of the vertices
    std::vector<Vec4d> m_verticesTangents; ///> Tangents of the vertices
};
}

#endif // ifndef imstkSurfaceMesh_h
