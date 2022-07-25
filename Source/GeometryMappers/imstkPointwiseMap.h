/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryMap.h"
#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkTypes.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class PointwiseMap
///
/// \brief PointwiseMap can compute & apply a mapping between parent and
/// child PointSet geometries.
///
class PointwiseMap : public GeometryMap
{
public:
    PointwiseMap();
    PointwiseMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child);
    ~PointwiseMap() override = default;

    IMSTK_TYPE_NAME(PointwiseMap)

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void compute() override;

    ///
    /// \brief Compute tet vertex id to surf vertex id map
    ///
    void computeMap(std::unordered_map<int, int>& tetVertToSurfVertMap);

    ///
    /// \brief Sets the one-to-one correspondence directly
    ///
    void setMap(const std::unordered_map<int, int>& sourceMap);
    const std::unordered_map<int, int>& getMap() const { return m_oneToOneMap; }

    ///
    /// \brief Get the mapped/corresponding parent index, given a child index.
    /// returns -1 if no correspondence found.
    /// \param index on the child geometry
    ///
    int getParentVertexId(const int childVertexId) const;

    ///
    /// \brief Set/Get the tolerance. The distance to consider
    /// two points equivalent/corresponding
    ///@{
    void setTolerance(const double tolerance) { m_epsilon = tolerance; }
    double getTolerance() const { return m_epsilon; }
///@}

protected:
    ///
    /// \brief Returns the first matching vertex, -1 if not found
    ///
    int findMatchingVertex(const VecDataArray<double, 3>& parentMesh, const Vec3d& p);

    ///
    /// \brief Apply (if active) the tetra-triangle mesh map
    ///
    void requestUpdate() override;

public:
    // A map and vector are maintained. The vector for parallel processing, the map for fast lookup
    std::unordered_map<int, int>     m_oneToOneMap;       ///< One to one mapping data
    std::vector<std::pair<int, int>> m_oneToOneMapVector; ///< One to one mapping data

    ///
    /// \brief Tolernace for considering two points equivalent/mapped.
    /// The tolerance is set a bit higher here since 32bit float PointSets are very
    /// commonly mapped to 64 bit double PointSets (because of file formats/IO)
    ///
    double m_epsilon = 0.00000001;
};
} // namespace imstk