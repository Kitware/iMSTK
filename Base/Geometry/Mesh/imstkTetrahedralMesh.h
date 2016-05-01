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

#ifndef imstkTetrahedralMesh_h
#define imstkTetrahedralMesh_h

#include <set>
#include <list>
#include <iostream>

#include "imstkVolumetricMesh.h"

namespace imstk {

///
/// \class TetrahedralMesh
///
/// \brief Tetrahedral mesh
///
class TetrahedralMesh : public VolumetricMesh
{
public:

    using TetraArray = std::array<size_t, 4>;
    using WeightsArray = std::array<double, 4>;

    TetrahedralMesh() : VolumetricMesh(GeometryType::TetrahedralMesh) {}

    ~TetrahedralMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  tetrahedra connectivity
    ///
    void initialize(const std::vector<Vec3d>& vertices,
                    const std::vector<TetraArray>& tetrahedra,
                    bool computeAttachedSurfaceMesh = false);

    ///
    /// \brief Clear all the mesh data
    ///
    void clear();

    ///
    /// \brief Print the tetrahedral mesh
    ///
    void print() const override;

    ///
    /// \brief Compute and return the volume of the tetrahedral mesh
    ///
    double getVolume() const override;

    ///
    /// \brief This method
    /// (a) Extracts the confirming triangular mesh from the tetrahedral mesh
    /// (b) Checks and flips the triangle connectivity order if it is not consistent
    /// (c) Renumbers the vertices
    /// TODO: OPTMIZE FOR SPEED
    bool extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh) override;

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the tetrahedra
    ///
    void computeBarycentricWeights(const size_t& tetId, const Vec3d& pos, WeightsArray& weights) const;

    ///
    /// \brief Compute the bounding box of a given tetrahedron
    ///
    void computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const;

    // Accessors

    ///
    /// \brief set the vector of array of IDs for the mesh
    ///
    void setTetrahedraVertices(const std::vector<TetraArray>& tetrahedrons);

    ///
    /// \brief Return the vector of array of IDs for all the tetrahedra
    ///
    const std::vector<TetraArray>& getTetrahedraVertices() const;

    ///
    /// \brief Return the array of IDs for a given tetrahedron
    ///
    const TetraArray& getTetrahedronVertices(const size_t& tetId) const;

    ///
    /// \brief Returns the number of tetrahedra
    ///
    int getNumTetrahedra() const;

protected:

    std::vector<TetraArray> m_tetrahedraVertices; ///< vertices of the tetrahedra
};
}

#endif // ifndef imstkTetrahedralMesh_h
