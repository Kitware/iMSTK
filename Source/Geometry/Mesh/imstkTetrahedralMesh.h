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

#include "imstkVolumetricMesh.h"

#include <array>

namespace imstk
{
class SurfaceMesh;

///
/// \class TetrahedralMesh
///
/// \brief Represents a set of tetrahedrons & vertices via an array of
/// Vec3d double vertices & Vec4i integer indices
///
class TetrahedralMesh : public VolumetricMesh
{
public:
    TetrahedralMesh(const std::string& name = std::string(""));
    ~TetrahedralMesh() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const override { return "TetrahedralMesh"; }

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  tetrahedra connectivity
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                    std::shared_ptr<VecDataArray<int, 4>> tetrahedra);

    ///
    /// \brief Clear all the mesh data
    ///
    void clear() override;

    ///
    /// \brief Print the tetrahedral mesh
    ///
    void print() const override;

    ///
    /// \brief This method extracts the conforming triangular mesh from the tetrahedral mesh
    ///
    std::shared_ptr<SurfaceMesh> extractSurfaceMesh() override;

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the tetrahedra
    ///
    Vec4d computeBarycentricWeights(const size_t& tetId, const Vec3d& pos) const;

    ///
    /// \brief Compute the bounding box of a given tetrahedron
    ///
    void computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const;

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const override { return true; }

// Accessors
    ///
    /// \brief set the vector of array of IDs for the mesh
    ///
    void setTetrahedraIndices(std::shared_ptr<VecDataArray<int, 4>> indices) { m_tetrahedraIndices = indices; }

    ///
    /// \brief Return the vector of array of IDs for all the tetrahedra
    ///
    std::shared_ptr<VecDataArray<int, 4>> getTetrahedraIndices() const { return m_tetrahedraIndices; }

    ///
    /// \brief Return the array of IDs for a given tetrahedron
    ///@{
    const Vec4i& getTetrahedronIndices(const size_t tetId) const;
    Vec4i& getTetrahedronIndices(const size_t tetId);
    ///@}

    ///
    /// \brief Returns the number of tetrahedra
    ///
    int getNumTetrahedra() const;

    ///
    /// \brief Get/set method for removed elements from the mesh
    ///@{
    void setTetrahedraAsRemoved(const unsigned int tetId) { m_removedMeshElems[tetId] = true; }
    const std::vector<bool>& getRemovedTetrahedra() const { return m_removedMeshElems; }
    ///@}

    ///
    /// \brief Compute and return the volume of the tetrahedral mesh
    ///
    double getVolume() override;

protected:
    std::shared_ptr<VecDataArray<int, 4>> m_tetrahedraIndices;

    std::vector<bool> m_removedMeshElems;
};
} // namespace imstk
