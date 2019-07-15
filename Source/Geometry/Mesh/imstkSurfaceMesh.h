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

#pragma once

// std library
#include <array>
#include <list>
#include <set>
#include <unordered_set>
#include <memory>

// imstk
#include "imstkPointSet.h"

namespace imstk
{
///
/// \brief Helper class for indentifying duplicate points
///
struct NormalGroup
{
    Vec3d position;
    Vec3d normal;
};
}

// This method is defined to allow for the map to be properly indexed by Texture objects
namespace std
{
template<> struct less<imstk::NormalGroup>
{
    bool operator()(const imstk::NormalGroup& group1,
                    const imstk::NormalGroup& group2) const
    {
        if (group1.position != group2.position)
        {
            return (group1.position.x() < group2.position.x());
        }

        if (group1.normal != group2.normal)
        {
            return (group1.normal.x() < group2.normal.x());
        }

        return false;
    }
};
}

namespace imstk
{
///
/// \class SurfaceMesh
///
/// \brief Surface triangular mesh
///
class SurfaceMesh : public PointSet
{
public:

    using TriangleArray = std::array<size_t, 3>;
    using NeighborsType = std::set<size_t>;

    ///
    /// \brief Constructor
    ///
    SurfaceMesh() : PointSet(Geometry::Type::SurfaceMesh) {}

    ///
    /// \brief Default destructor
    ///
    ~SurfaceMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  triangle connectivity
    ///
    void initialize(const StdVectorOfVec3d&           vertices,
                    const std::vector<TriangleArray>& triangles,
                    const bool                        computeDerivedData = false);

    ///
    /// \brief Initializes the rest of the data structures given vertex positions,
    ///  triangle connectivity, and normals
    ///
    void initialize(const StdVectorOfVec3d&           vertices,
                    const std::vector<TriangleArray>& triangles,
                    const StdVectorOfVec3d&           normals,
                    const bool                        computeDerivedData = false);

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
    void computeVertexNeighborTriangles();

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    void computeVertexNeighborVertices();

    ///
    /// \brief Compute the normals to the triangles
    ///
    void computeTrianglesNormals();

    ///
    /// \brief Computes the normals of all the vertices
    ///
    void computeVertexNormals();

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
    const StdVectorOfVec3d& getTriangleNormals() const;

    ///
    /// \brief Get normal of a triangle given its index
    ///
    const Vec3d& getTriangleNormal(size_t i) const;

    ///
    /// \brief Set/Get vector of normals of all the vertices
    ///
    void setVertexNormals(const StdVectorOfVec3d& normals);
    const StdVectorOfVec3d& getVertexNormals() const;

    ///
    /// \brief Set/Get vector of tangents of all the vertices
    ///
    void setVertexTangents(const StdVectorOfVec3d& tangents);
    const StdVectorOfVec3d& getVertexTangents() const;

    ///
    /// \brief Returns the number of triangles
    ///
    size_t getNumTriangles() const;

    ///
    /// \brief Set/Get the array defining the default material coordinates
    ///
    void setDefaultTCoords(std::string arrayName);
    std::string getDefaultTCoords();

    ///
    /// \brief Flip the normals for the whole mesh by reversing the winding order
    ///
    void flipNormals();

    ///
    /// \brief Enforces consistency in the winding order of the triangles
    ///
    void correctWindingOrder();

    ///
    /// \brief Finds vertices along vertex seams that share geometric properties
    ///
    void computeUVSeamVertexGroups();

    ///
    /// \brief Set load factor
    /// \param loadFactor the maximum number of vertices; a multiple of the original vertex count
    ///
    virtual void setLoadFactor(double loadFactor);

    ///
    /// \brief Get the maximum number of triangles
    ///
    size_t getMaxNumTriangles();

    ///
    /// \brief Returns the mesh graph
    ///
    std::shared_ptr<Graph> getMeshGraph() override;

protected:

    friend class VTKSurfaceMeshRenderDelegate;

    ///
    /// \brief Get vertex normals
    ///
    StdVectorOfVec3d& getVertexNormalsNotConst();

    std::vector<TriangleArray> m_trianglesVertices;       ///> Triangle connectivity

    std::vector<NeighborsType> m_vertexNeighborTriangles; ///> Neighbor triangles to vertices
    std::vector<NeighborsType> m_vertexNeighborVertices;  ///> Neighbor vertices to vertices

    StdVectorOfVec3d m_triangleNormals;                   ///> Normals to the triangles
    StdVectorOfVec3d m_triangleTangents;                  ///> Tangents to the triangles
    StdVectorOfVec3d m_vertexNormals;                     ///> Normals of the vertices
    StdVectorOfVec3d m_vertexTangents;                    ///> Tangents of the vertices

    std::map<NormalGroup, std::shared_ptr<std::vector<size_t>>> m_UVSeamVertexGroups;

    std::string m_defaultTCoords = ""; ///> Name of the array used as default material coordinates

    size_t m_originalNumTriangles = 0;
    size_t m_maxNumTriangles      = 0;
};
} // imstk
