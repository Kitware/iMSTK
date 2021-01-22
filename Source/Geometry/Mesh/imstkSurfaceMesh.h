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

#include "imstkPointSet.h"

#include <array>
#include <set>

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

namespace std
{
///
/// \struct less
/// \brief This method is defined to allow for the map to be properly indexed by Texture objects
///
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
    using NeighborsType = std::set<size_t>;

public:
    ///
    /// \brief Constructor
    ///
    SurfaceMesh(const std::string& name = std::string(""));

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  triangle connectivity
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                    std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                    const bool computeDerivedData = false);

    ///
    /// \brief Initializes the rest of the data structures given vertex positions,
    ///  triangle connectivity, and normals
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                    std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                    std::shared_ptr<VecDataArray<double, 3>> normals,
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
    /// \brief Computes neighboring triangles for all vertices
    ///
    void computeVertexNeighborTriangles();

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    void computeVertexNeighborVertices();

    ///
    /// \brief Compute the normals of all the triangles
    ///
    void computeTrianglesNormals();

    ///
    /// \brief Compute the tangents of all the triangles
    /// based off
    ///
    void computeTriangleTangents();

    ///
    /// \brief Computes the normals of all the vertices
    ///
    void computeVertexNormals();

    ///
    /// \brief Comptues the tangents of all the vertices
    ///
    void computeVertexTangents();

    ///
    /// \brief Rewire the node order and triangle connectivity to optimize for memory layout
    ///  The intended use is for large meshes that doesn't fit into CPU/GPU memory.
    ///  \todo Further optimization to find a 1-d uninterrupted sub-graph at each iteration.
    ///
    void optimizeForDataLocality();

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
    /// \brief Copy the contents of one SurfaceMesh to the other (no pointers to shared data between this and srcMesh)
    /// \todo: generalize base classes and implement for every geometry
    ///
    void deepCopy(std::shared_ptr<SurfaceMesh> srcMesh);

// Accessors
public:
    ///
    /// \brief Get/Set triangle connectivity
    ///
    void setTriangleIndices(std::shared_ptr<VecDataArray<int, 3>> indices) { m_triangleIndices = indices; }
    std::shared_ptr<VecDataArray<int, 3>> getTriangleIndices() const { return m_triangleIndices; }

    ///
    /// \brief Return connectivity of a triangle
    ///
    const Vec3i& getTriangleIndices(const int triangleNum) const;
    Vec3i& getTriangleIndices(const int triangleNum);

    ///
    /// \brief Returns the number of triangles
    ///
    size_t getNumTriangles() const;

    ///
    /// \brief Get the maximum number of triangles
    ///
    size_t getMaxNumTriangles() const { return m_maxNumTriangles; }

    ///
    /// \brief Get the volume enclosed by the surface mesh
    ///
    double getVolume() override;

// Attributes
public:
    ///
    /// \brief Set a data array holding some per cell data
    ///
    void setCellAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr);

    ///
    /// \brief Get a specific data array. If the array name cannot be found, nullptr is returned.
    ///
    std::shared_ptr<AbstractDataArray> getCellAttribute(const std::string& arrayName) const;

    ///
    /// \brief Get the cell attributes map
    ///
    const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& getCellAttributes() const { return m_cellAttributes; }

    ///
    /// \brief Check if a specific data array exists.
    ///
    bool hasCellAttribute(const std::string& arrayName) const;

    ///
    /// \brief Set the cell attributes map
    ///
    void setCellAttributes(std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes) { m_cellAttributes = attributes; }

    ///
    /// \brief Get/Set the active scalars
    ///
    void setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars);
    void setCellScalars(const std::string& arrayName);
    std::string getActiveCellScalars() const { return m_activeCellScalars; }
    std::shared_ptr<AbstractDataArray> getCellScalars() const;

    ///
    /// \brief Get/Set the active normals
    ///
    void setCellNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals);
    void setCellNormals(const std::string& arrayName);
    std::string getActiveCellNormals() const { return m_activeCellNormals; }
    std::shared_ptr<VecDataArray<double, 3>> getCellNormals() const;

    ///
    /// \brief Get/Set the active tangents
    ///
    void setCellTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> tangents);
    void setCellTangents(const std::string& arrayName);
    std::string getActiveCellTangents() const { return m_activeCellTangents; }
    std::shared_ptr<VecDataArray<double, 3>> getCellTangents() const;

protected:
    std::shared_ptr<VecDataArray<int, 3>> m_triangleIndices;
    std::vector<NeighborsType> m_vertexNeighborTriangles; ///> Neighbor triangles to vertices
    std::vector<NeighborsType> m_vertexNeighborVertices;  ///> Neighbor vertices to vertices

    std::map<NormalGroup, std::shared_ptr<std::vector<size_t>>> m_UVSeamVertexGroups;

    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> m_cellAttributes;
    std::string m_activeCellNormals  = "";
    std::string m_activeCellTangents = "";
    std::string m_activeCellScalars  = "";

    size_t m_originalNumTriangles = 0;
    size_t m_maxNumTriangles      = 0;
};
} // imstk
