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

#include "imstkVolumetricMesh.h"

namespace imstk {
class TetrahedralMesh : public VolumetricMesh
{
    using TetraArray = std::array<size_t, 4>;

public:

    TetrahedralMesh() : VolumetricMesh(GeometryType::TetrahedralMesh) {}

    ~TetrahedralMesh() = default;

    // Accessors
    const std::vector<TetraArray>& getTetrahedronVertices() const;
    void setTetrahedronVertices(const std::vector<TetraArray>& tetrahedrons);

    ///
    /// \brief Returns the number of tetrahedra
    ///
    int getNumTetrahedra() const;

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the tetrahedra
    ///
    void computeBarycentricWeights(const int closestEle, const imstk::Vec3d& p, std::array<double, 4> weights) const;

    ///
    /// \brief get the indices vertices of a given tetrahedra in an array
    ///
    const TetraArray& getTetrahedronVertices(const int tetraNum) const;

    ///
    /// \brief Compute the bounding box of a given tetrahedra
    ///
    void computeTetrahedraBoundingBox(imstk::Vec3d& min, imstk::Vec3d& max, const int tetNum) const;

    ///
    /// \brief Compute and return the volume of the tetrahedral mesh
    ///
    double getVolume() const;

protected:

    std::vector<TetraArray> m_tetrahedronVertices; ///< vertices of the tetrahedra
};
}

#endif // ifndef imstkTetrahedralMesh_h
