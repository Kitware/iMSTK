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

// imstk
#include "imstkGeometryMap.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
///
/// \class OneToOneMap
///
/// \brief Computes and applies the One-to-one map. The master and the slave geometries
/// should contain nodes.
///
class OneToOneMap : public GeometryMap
{
public:
    ///
    /// \brief Constructor
    ///
    OneToOneMap() : GeometryMap(GeometryMap::Type::OneToOne){}

    ///
    /// \brief Default destructor
    ///
    virtual ~OneToOneMap() override = default;

    ///
    /// \brief Compute the tetra-triangle mesh map
    ///
    void compute() override;

    ///
    /// \brief Check the validity of the map
    ///
    bool isValid() const override;

    ///
    /// \brief Sets the one-to-one correspondence directly
    ///
    void setMap(const std::map<size_t, size_t>& sourceMap);

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

    ///
    /// \brief
    ///
    size_t getMapIdx(const size_t& idx) override { return m_oneToOneMap[idx]; }

protected:
    std::map<size_t, size_t> m_oneToOneMap; ///> One to one mapping data

    // This vector is for parallel processing, it should contain identical data as m_oneToOneMap
    std::vector<std::pair<size_t, size_t>> m_oneToOneMapVector; ///> One to one mapping data
};
} // imstk

#endif // imstkOneToOneMap_h
