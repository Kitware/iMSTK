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

#include "imstkGeometry.h"

namespace imstk
{
///
/// \class GeometryMap
///
/// \brief Base class for any geometric map
///
class GeometryMap
{
protected:
    GeometryMap() : m_isActive(true) { }

public:
    virtual ~GeometryMap() = default;

    ///
    /// \brief Returns the string class name
    ///
    /// Required for using IMSTK_TYPE_MACRO
    virtual const std::string getTypeName() const = 0;

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
    /// \brief Get/Set parent geometry
    ///@{
    virtual void setParentGeometry(std::shared_ptr<Geometry> parent);
    virtual std::shared_ptr<Geometry> getParentGeometry() const;
    ///@}

    ///
    /// \brief Get/Set child geometry
    ///@{
    virtual void setChildGeometry(std::shared_ptr<Geometry> child);
    virtual std::shared_ptr<Geometry> getChildGeometry() const;
    ///@}

    ///
    /// \brief Initialize the map
    ///
    virtual void initialize();

protected:
    bool m_isActive;                        ///> true if the map us active at runtime

    std::shared_ptr<Geometry> m_parentGeom; ///> the geometry which dictates the configuration
    std::shared_ptr<Geometry> m_childGeom;  ///> the geometry which follows the parent
};
} // namespace imstk
