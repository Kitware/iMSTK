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

#include "imstkMacros.h"
#include "imstkVolumetricMesh.h"
#include "imstkVecDataArray.h"

#include <array>

namespace imstk
{
///
/// \class TetrahedralMesh
///
/// \brief Represents a set of tetrahedrons & vertices via an array of
/// Vec3d double vertices & Vec4i integer indices
///
class TetrahedralMesh : public VolumetricMesh<4>
{
public:
    TetrahedralMesh() = default;
    ~TetrahedralMesh() override = default;

    IMSTK_TYPE_NAME(TetrahedralMesh)

    ///
    /// \brief This method extracts the conforming triangular mesh from the tetrahedral mesh
    ///
    std::shared_ptr<SurfaceMesh> extractSurfaceMesh() override;

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the tetrahedra
    ///
    Vec4d computeBarycentricWeights(const int tetId, const Vec3d& pos) const override;

    ///
    /// \brief Compute the bounding box of a given tetrahedron
    ///
    void computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const;

// Accessors
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

    int getNumTetrahedra() const { return getNumCells(); }
    void setTetrahedraIndices(std::shared_ptr<VecDataArray<int, 4>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 4>> getTetrahedraIndices() const { return getCells(); }

protected:
    std::vector<bool> m_removedMeshElems;
};
} // namespace imstk
