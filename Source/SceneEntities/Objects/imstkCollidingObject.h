/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"

namespace imstk
{
class Geometry;
class GeometryMap;

///
/// \class CollidingObject
///
/// \brief A SceneObject with a geometry for collision
///
class CollidingObject : public SceneObject
{
public:
    CollidingObject(const std::string& name = "CollidingObject") : SceneObject(name) { }
    ~CollidingObject() override = default;

    IMSTK_TYPE_NAME(CollidingObject)

    ///
    /// \brief
    ///@{
    std::shared_ptr<Geometry> getCollidingGeometry() const;
    void setCollidingGeometry(std::shared_ptr<Geometry> geometry);
    ///@}

    ///
    /// \brief
    ///@{
    std::shared_ptr<GeometryMap> getCollidingToVisualMap() const;
    void setCollidingToVisualMap(std::shared_ptr<GeometryMap> map);
    ///@}

    ///
    /// \brief Updates the geometries from the maps (if defined)
    ///
    void updateGeometries() override;

    ///
    /// \brief Initialize the scene object
    ///
    bool initialize() override;

protected:
    std::shared_ptr<Geometry>    m_collidingGeometry    = nullptr; ///< Geometry for collisions
    std::shared_ptr<GeometryMap> m_collidingToVisualMap = nullptr; ///< Maps transformations to visual geometry
    Vec3d m_force = Vec3d::Zero();
};
} // namespace imstk