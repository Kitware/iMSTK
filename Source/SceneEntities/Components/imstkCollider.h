/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkEventObject.h"

#include <memory>
#include <string>

namespace imstk
{
class Geometry;
class GeometryMap;

///
/// \class Collider
///
/// \brief Contains geometry for collision.
///
class Collider : public Component
{
public:
    Collider(const std::string& name = "Collider")
      : Component(name)
    {
    }

    ~Collider() override = default;

    ///
    /// \brief Get/Set geometry
    ///@{
    std::shared_ptr<Geometry> getGeometry() const { return m_geometry; }
    void setGeometry(std::shared_ptr<Geometry> geometry) { m_geometry = geometry; }
    ///@}

    ///
    /// \brief Get/Set name
    ///@{
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = name; }
    ///@}

    static std::shared_ptr<Geometry> getCollidingGeometryFromEntity(Entity* entity);

protected:
    std::shared_ptr<Geometry> m_geometry; ///< Geometry for collisions
};
} // namespace imstk
