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

#include "imstkCellMesh.h"
#include "imstkMacros.h"

#include <array>
#include <unordered_set>

namespace imstk
{
///
/// \struct NormalGroup
///
/// \brief Helper class for indentifying duplicate points
///
struct NormalGroup
{
    Vec3d position;
    Vec3d normal;
};
} // namespace imstk

namespace imstk
{
///
/// \brief Utility for triangle comparison
///
struct TriCell
{
    std::array<std::uint32_t, 3> vertexIds;

    TriCell(std::uint32_t id0, std::uint32_t id1, std::uint32_t id2)
    {
        vertexIds[0] = id0;
        vertexIds[1] = id1;
        vertexIds[2] = id2;
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
        if (vertexIds[1] > vertexIds[2])
        {
            std::swap(vertexIds[1], vertexIds[2]);
        }
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
    }

    // Test true equivalence
    bool operator==(const TriCell& other) const
    {
        // Only works if sorted
        return (vertexIds[0] == other.vertexIds[0] && vertexIds[1] == other.vertexIds[1]
                && vertexIds[2] == other.vertexIds[2]);
    }
};
} // namespace imstk

namespace std
{
///
/// \struct hash<imstk::TriCell>
///
/// \brief Gives a hashing function for triangles that results in
/// identical hashes for any ordering of ids hash(0,1,2)=hash(1,2,0)
/// and is well distributed (avoids collisions)
///
template<>
struct hash<imstk::TriCell>
{
    // A 128 int could garuntee no collisions but its harder to find support for
    std::size_t operator()(const imstk::TriCell& k) const
    {
        using std::size_t;
        using std::hash;

        // Assuming sorted
        const std::size_t r =
            imstk::symCantor(static_cast<size_t>(k.vertexIds[0]), static_cast<size_t>(k.vertexIds[1]));
        return imstk::symCantor(r, static_cast<size_t>(k.vertexIds[2]));
    }
};
///
/// \struct less<imstk::NormalGroup>
///
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
} // namespace std

namespace imstk
{
///
/// \class SurfaceMesh
///
/// \brief Represents a set of triangles & vertices via an array of
/// Vec3d double vertices & Vec3i integer indices
///
class SurfaceMesh : public CellMesh<3>
{
public:
    SurfaceMesh() = default;
    ~SurfaceMesh() override = default;

    IMSTK_TYPE_NAME(SurfaceMesh)

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
    /// \brief Get the volume enclosed by the surface mesh
    ///
    double getVolume() override;

    int getNumTriangles() const { return getNumCells(); }
    void setTriangleIndices(std::shared_ptr<VecDataArray<int, 3>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 3>> getTriangleIndices() const { return getCells(); }

protected:
    std::map<NormalGroup, std::shared_ptr<std::vector<size_t>>> m_UVSeamVertexGroups;
};
} // namespace imstk