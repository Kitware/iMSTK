/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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

    IMSTK_TYPE_NAME(PointToTetMap)

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
    std::vector<Vec4d> m_verticesWeights;        ///< weights

    std::vector<int> m_verticesEnclosingTetraId; ///< Enclosing tetrahedra to interpolate the weights upon

    std::vector<Vec3d> m_bBoxMin;
    std::vector<Vec3d> m_bBoxMax;
    bool m_boundingBoxAvailable;

private:
    std::shared_ptr<VecDataArray<double, 3>> m_childVerts;
};
} // namespace imstk