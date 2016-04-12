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

#ifndef imstkTetraTriangleMap_h
#define imstkTetraTriangleMap_h

#include <limits>

#include "imstkGeometryMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk {

///
/// \class TetraTriangleMap
///
/// \brief Computes and applies the triangle-tetrahedra map. The master mesh is the
///  tetrahedral mesh and the slave is the surface triangular mesh.
///
class TetraTriangleMap : public GeometryMap
{
    using weightsArray = std::array<double, 4>;

public:

    ~TetraTriangleMap() = default;

    TetraTriangleMap() : GeometryMap(GeometryMapType::TetraTriangle){}

    ///
    /// \brief Apply (if active) the tetra-triangle mesh map
    ///
    void applyMap();

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void computeMap();

    // Generic utility functions

    ///
    /// \brief Find the closest tetrahedra based on the distance to their centroids for a given point in 3D space
    ///
    static int findClosestTetrahedra(const std::shared_ptr<imstk::TetrahedralMesh> tetraMesh, const imstk::Vec3d& p);

    ///
    /// \brief Find the tetrahedra that encloses a given point in 3D space
    ///
    static int findEclosingTetrahedra(const std::shared_ptr<imstk::TetrahedralMesh> tetraMesh, const imstk::Vec3d& p);

    // Accessors

    ///
    /// \brief Set the geometry that dictates the map
    ///
    void setMaster(std::shared_ptr<Geometry> master) override;

    ///
    /// \brief Set the geometry that follows the master
    ///
    void setSlave(std::shared_ptr<Geometry> slave) override;

    ///
    /// \brief Print the map
    ///
    void printMap() const;

protected:
    std::vector<weightsArray> m_weights; ///> weights
    std::vector<int> m_enclosingTetra; ///> Enclosing tetrahedra to interpolate the weights upon
};
}

#endif // imstkTetraTriangleMap_h
