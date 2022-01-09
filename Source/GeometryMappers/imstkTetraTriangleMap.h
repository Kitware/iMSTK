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

#include "imstkGeometryMap.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class TetraTriangleMap
///
/// \brief Computes and applies the triangle-tetrahedra map. The parent mesh is the
///  tetrahedral mesh and the child is the surface triangular mesh.
///
class TetraTriangleMap : public GeometryMap
{
public:
    TetraTriangleMap() : m_boundingBoxAvailable(false) { }
    TetraTriangleMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child)
        : m_boundingBoxAvailable(false)
    {
        this->setParentGeometry(parent);
        this->setChildGeometry(child);
    }

    virtual ~TetraTriangleMap() override = default;

    virtual const std::string getTypeName() const override { return "TetraTriangleMap"; }

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
    void setParentGeometry(std::shared_ptr<Geometry> parent) override;

    ///
    /// \brief Set the geometry that follows the parent
    ///
    void setChildGeometry(std::shared_ptr<Geometry> child) override;

protected:
    ///
    /// \brief Find the tetrahedron that encloses a given point in 3D space
    ///
    size_t findEnclosingTetrahedron(const Vec3d& pos) const;

    ///
    /// \brief Update bounding box of each tetrahedra of the mesh
    ///
    void updateBoundingBox();

    ///
    /// \brief Find the closest tetrahedron based on the distance to their centroids for a given
    /// point in 3D space
    ///
    size_t findClosestTetrahedron(const Vec3d& pos) const;

    std::vector<Vec4d> m_verticesWeights;           ///> weights

    std::vector<size_t> m_verticesEnclosingTetraId; ///> Enclosing tetrahedra to interpolate the weights upon

    std::vector<Vec3d> m_bBoxMin;
    std::vector<Vec3d> m_bBoxMax;
    bool m_boundingBoxAvailable;

private:
    std::shared_ptr<VecDataArray<double, 3>> m_childVerts;
};
}  // namespace imstk
