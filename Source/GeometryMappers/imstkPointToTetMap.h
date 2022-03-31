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
#include "imstkMacros.h"
#include "imstkMath.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class PointToTetMap
///
/// \brief Computes and applies the PointSet-Tetrahedra map. Vertices of the
/// child geometry are deformed according to the deformation of the tetrahedron
/// they are located in. If they are not within one, nearest tet is used.
///
class PointToTetMap : public GeometryMap
{
public:
    PointToTetMap();
    PointToTetMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child);
    ~PointToTetMap() override = default;

    IMSTK_TYPE_NAME(TetraTriangleMap)

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void compute() override;

protected:
    ///
    /// \brief Apply (if active) the tetra-triangle mesh map
    ///
    void requestUpdate() override;

    ///
    /// \brief Find the tetrahedron that encloses a given point in 3D space
    ///
    int findEnclosingTetrahedron(const Vec3d& pos) const;

    ///
    /// \brief Update bounding box of each tetrahedra of the mesh
    ///
    void updateBoundingBox();

    ///
    /// \brief Find the closest tetrahedron based on the distance to their centroids for a given
    /// point in 3D space
    ///
    int findClosestTetrahedron(const Vec3d& pos) const;

protected:
    std::vector<Vec4d> m_verticesWeights;        ///> weights

    std::vector<int> m_verticesEnclosingTetraId; ///> Enclosing tetrahedra to interpolate the weights upon

    std::vector<Vec3d> m_bBoxMin;
    std::vector<Vec3d> m_bBoxMax;
    bool m_boundingBoxAvailable;

private:
    std::shared_ptr<VecDataArray<double, 3>> m_childVerts;
};
} // namespace imstk