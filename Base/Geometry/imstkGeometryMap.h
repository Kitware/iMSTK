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

#ifndef imstkGeometryMap_h
#define imstkGeometryMap_h

#include <memory>

#include "imstkGeometry.h"
#include "g3log/g3log.hpp"

namespace imstk {

enum class GeometryMapType
{
    Isometric,
    Identity,
    TetraTriangle,
    HexaTriangle,
    TetraTetra
};

class GeometryMap
{
public:

    ~GeometryMap() = default;

    ///
    /// \brief Compute the map
    ///
    virtual void computeMap() = 0;

    ///
    /// \brief Apply the map
    ///
    virtual void applyMap() = 0;

    ///
    /// \brief Deactivate the map
    ///
    void muteMap();

    ///
    /// \brief Activate the map
    ///
    void activateMap();

    // Accessors
    const GeometryMapType& getType() const;

    virtual void setMaster(std::shared_ptr<Geometry> master);
    virtual std::shared_ptr<Geometry> getMaster() const;

    virtual void setSlave(std::shared_ptr<Geometry> slave);
    virtual std::shared_ptr<Geometry> getSlave() const;

    ///
    /// \brief Returns true if the map is actively applied at runtime, else false.
    ///
    bool isActive() const;

protected:

    GeometryMap(GeometryMapType type) : m_isActive(true) {}

    GeometryMapType m_type; ///> type of the map

    bool m_isActive; ///> true if the map us active at runtime

    std::shared_ptr<Geometry> m_master; ///> the geometry which dictates the configuration
    std::shared_ptr<Geometry> m_slave; ///> the geometry which follows the master

};
}

#endif // imstkGeometryMap_h
