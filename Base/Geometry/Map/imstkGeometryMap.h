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
#include <iostream>

#include "imstkGeometry.h"
#include "g3log/g3log.hpp"

namespace imstk
{

///
/// \class GeometryMap
///
/// \brief Base class for any geometric map
///
class GeometryMap
{
public:

    enum class Type
    {
        Isometric,
        Identity,
        OneToOne,
        TetraTriangle,
        HexaTriangle,
        TetraTetra
    };

    ///
    /// \brief Destructor
    ///
    ~GeometryMap() = default;

    ///
    /// \brief Compute the map
    ///
    virtual void compute() = 0;

    ///
    /// \brief Apply the map
    ///
    virtual void apply() = 0;

    ///
    /// \brief Print the map
    ///
    virtual void print() const;

    ///
    /// \brief Deactivate the map
    ///
    void mute();

    ///
    /// \brief Check the validity of the map
    ///
    virtual bool isValid() const = 0;

    ///
    /// \brief Activate the map
    ///
    void activate();

    ///
    /// \brief Returns true if the map is actively applied at runtime, else false.
    ///
    bool isActive() const;

    // Accessors

    ///
    /// \brief Returns the type of the map
    ///
    const Type &getType() const;

    ///
    /// \brief Returns the string representing the type name of the map
    ///
    const std::string getTypeName() const;

    ///
    /// \brief Get/Set master geometry
    ///
    virtual void setMaster(std::shared_ptr<Geometry> master);
    virtual std::shared_ptr<Geometry> getMaster() const;

    ///
    /// \brief Get/Set slace geometry
    ///
    virtual void setSlave(std::shared_ptr<Geometry> slave);
    virtual std::shared_ptr<Geometry> getSlave() const;

    ///
    /// \brief getMapIdx
    /// \param idx
    /// \return index of Master corresponding to the idx of Slave
    ///
    virtual unsigned int getMapIdx(const unsigned int& idx)
    {
        return 0;
    }
protected:

    ///
    /// \brief Protected constructor
    ///
    GeometryMap(Type type) : m_type(type), m_isActive(true) {}

    Type m_type;     ///> type of the map
    bool m_isActive; ///> true if the map us active at runtime

    std::shared_ptr<Geometry> m_master; ///> the geometry which dictates the configuration
    std::shared_ptr<Geometry> m_slave;  ///> the geometry which follows the master

};

}

#endif // imstkGeometryMap_h
