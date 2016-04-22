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

#ifndef imstkOneToOneMap_h
#define imstkOneToOneMap_h

#include <limits>

#include "imstkGeometryMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk {

///
/// \class OneToOneMap
///
/// \brief Computes and applies the One-to-one map. The master and the slave geometries
/// should contain nodes.
///
class OneToOneMap : public GeometryMap
{
public:

    OneToOneMap() : GeometryMap(GeometryMapType::OneToOne){}

    ~OneToOneMap() = default;

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void compute() override;

    ///
    /// \brief Find the matching node
    ///
    int findMatchingVertex(std::shared_ptr<Mesh> tetraMesh, const Vec3d& p) const;

    ///
    /// \brief Check the validity of the map
    ///
    bool isValid() const;

    ///
    /// \brief Sets the one-to-one correspondence directly
    ///
    void setMap(const std::map<int, int>& sourceMap);

    ///
    /// \brief Apply (if active) the tetra-triangle mesh map
    ///
    void apply() override;

    ///
    /// \brief Print the map
    ///
    void print() const override;

    ///
    /// \brief Set the geometry that dictates the map
    ///
    void setMaster(std::shared_ptr<Geometry> master) override;

    ///
    /// \brief Set the geometry that follows the master
    ///
    void setSlave(std::shared_ptr<Geometry> slave) override;

protected:
    std::map<int, int> m_oneToOneMap;
};
}

#endif // imstkOneToOneMap_h
