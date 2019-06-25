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

#include <limits>

// imstk
#include "imstkGeometryMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
///
/// \class TetraTriangleMap
///
/// \brief Computes and applies the triangle-tetrahedra map. The master mesh is the
///  tetrahedral mesh and the slave is the surface triangular mesh.
///
class TetraTriangleMap : public GeometryMap
{
public:
    ///
    /// \brief Constructor
    ///
    TetraTriangleMap() : GeometryMap(GeometryMap::Type::TetraTriangle){}

    ///
    /// \brief Destructor
    ///
    ~TetraTriangleMap() = default;

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void compute() override;

    ///
    /// \brief Apply (if active) the tetra-triangle mesh map
    ///
    void apply() override;

    ///
    /// \brief Print the map
    ///
    void print() const override;

    ///
    /// \brief Check the validity of the map
    ///
    bool isValid() const override;

    ///
    /// \brief Set the geometry that dictates the map
    ///
    void setMaster(std::shared_ptr<Geometry> master) override;

    ///
    /// \brief Set the geometry that follows the master
    ///
    void setSlave(std::shared_ptr<Geometry> slave) override;

    ///
    /// \brief Find the closest tetrahedron based on the distance to their centroids for a given point in 3D space
    ///
    static size_t findClosestTetrahedron(std::shared_ptr<TetrahedralMesh> tetraMesh,
                                         const Vec3d& pos);

    ///
    /// \brief Find the tetrahedron that encloses a given point in 3D space
    ///
    static size_t findEnclosingTetrahedron(std::shared_ptr<TetrahedralMesh> tetraMesh,
                                           const Vec3d& pos);

protected:
    std::vector<TetrahedralMesh::WeightsArray> m_verticesWeights; ///> weights
    std::vector<size_t> m_verticesEnclosingTetraId; ///> Enclosing tetrahedra to interpolate the weights upon
};
} // imstk
