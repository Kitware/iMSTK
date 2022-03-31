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
#include "imstkTypes.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class OneToOneMap
///
/// \brief OneToOneMap can compute & apply a one-to-one mapping between parent
/// and child PointSet geometries.
///
class OneToOneMap : public GeometryMap
{
public:
    OneToOneMap();
    OneToOneMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child);
    ~OneToOneMap() override = default;

    IMSTK_TYPE_NAME(OneToOneMap)

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
    std::unordered_map<int, int>     m_oneToOneMap;       ///> One to one mapping data
    std::vector<std::pair<int, int>> m_oneToOneMapVector; ///> One to one mapping data

    double m_epsilon = IMSTK_DOUBLE_EPS;                  // Tolerance for considering two points equivalent
};
} // namespace imstk