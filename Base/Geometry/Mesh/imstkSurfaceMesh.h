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

// std library
#include <array>
#include <list>
#include <set>

// imstk
#include "imstkMesh.h"

namespace imstk
{

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

    ///
    /// \brief Constructor
    ///
    SurfaceMesh() : Mesh(Geometry::Type::SurfaceMesh) {}

    ///
    /// \brief Default destructor
    ///
    ~SurfaceMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  triangle connectivity and texture coordinates
    ///
    void initialize(const StdVectorOfVec3d& vertices,
                    const std::vector<TriangleArray>& triangles,
                    const bool computeDerivedData = false);

    ///
    /// \brief Clear all the mesh data
    ///
    void clear() override;

    ///
    /// \brief Print the surface mesh
    ///
    void print() const override;

    ///
    /// \brief Get the volume enclosed by the surface mesh
    ///
    double getVolume() const override;

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
    /// \brief Rewire the node order and triangle connectivity to optimize for memory layout
    ///  The intended use is for large meshes that doesn't fit into CPU/GPU memory.
    ///  TODO: Further optimization to find a 1-d uninterrupted sub-graph at each iteration.
    ///
    void optimizeForDataLocality();

    // Accessors

    ///
    /// \brief Get/Set triangle connectivity
    ///
    void setTrianglesVertices(const std::vector<TriangleArray>& triangles);
    const std::vector<TriangleArray>& getTrianglesVertices() const;

    ///
    /// \brief Get vector of normals of all the triangles
    ///
    const StdVectorOfVec3d& getTrianglesNormals() const;

    ///
    /// \brief Get normal of a triangle given its index
    ///
    const Vec3d& getTriangleNormal(size_t i) const;

    ///
    /// \brief Set/Get vector of normals of all the vertices
    ///
    void setVerticesNormals(const StdVectorOfVec3d& normals);
    const StdVectorOfVec3d& getVerticesNormals() const;

    ///
    /// \brief Set/Get vector of tangents of all the vertices
    ///
    void setVerticesTangents(const StdVectorOfVec3d& tangents);
    const StdVectorOfVec3d& getVerticesTangents() const;

    ///
    /// \brief Set/Get vector of bitangents of all the vertices
    ///
    void setVerticesBitangents(const StdVectorOfVec3d& bitangents);
    const StdVectorOfVec3d& getVerticesBitangents() const;

    ///
    /// \brief Get normal of a vertex given its index
    ///
    const Vec3d& getVerticeNormal(size_t i) const;

    ///
    /// \brief Returns the number of triangles
    ///
    int getNumTriangles() const;

    ///
    /// \brief Set/Get the array defining the default texture coordinates
    ///
    void setDefaultTCoords(std::string arrayName);
    std::string getDefaultTCoords();

    ///
    /// \brief Add texture by giving the texture file name and the texture coordinates array name
    ///
    void addTexture(std::string tFileName, std::string tCoordsName = "");
    const std::map<std::string, std::string>& getTextureMap() const;
    std::string getTexture(std::string tCoordsName) const;

protected:

    std::vector<TriangleArray> m_trianglesVertices; ///> Triangle connectivity

    std::vector<NeighborsType> m_verticesNeighborTriangles; ///> Neighbor triangles to vertices
    std::vector<NeighborsType> m_verticesNeighborVertices; ///> Neighbor vertices to vertices

    StdVectorOfVec3d m_trianglesNormals; ///> Normals to the triangles
    StdVectorOfVec3d m_verticesNormals; ///> Normals of the vertices
    StdVectorOfVec3d m_verticesTangents; ///> Tangents of the vertices
    StdVectorOfVec3d m_verticesBitangents; ///> Bitangents of the vertices

    std::string m_defaultTCoords = ""; ///> Name of the array used as default texture coordinates
    std::map<std::string, std::string> m_textureMap; ///> Mapping texture coordinates to texture
};

} // imstk

#endif // ifndef imstkSurfaceMesh_h
