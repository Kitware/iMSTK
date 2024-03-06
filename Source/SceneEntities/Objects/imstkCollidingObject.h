/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkCollisionData.h"

#include <unordered_map>

namespace imstk
{
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

    void update() override;

    /// \return Whether `other` collided with this object since the last \sa update() call
    /// \param other colliding object for the check
    bool didCollide(std::shared_ptr<CollidingObject> other);

    /// \brief Add data for collision with the object `other
    void addCollision(std::shared_ptr<CollidingObject> other, std::shared_ptr<imstk::CollisionData> data);

    const std::vector<std::shared_ptr<imstk::CollisionData>>& getCollisions(std::shared_ptr<CollidingObject> other) const;

protected:
    void postModifiedAll() override;

    /// \brief resets the collision information
    void clearCollisions();

    std::shared_ptr<Geometry>    m_collidingGeometry    = nullptr; ///< Geometry for collisions
    std::shared_ptr<GeometryMap> m_collidingToVisualMap = nullptr; ///< Maps transformations to visual geometry
    Vec3d m_force = Vec3d::Zero();

    std::unordered_map<std::shared_ptr<CollidingObject>, std::vector<std::shared_ptr<CollisionData>>> m_collisions;
};

std::string getCDType(const CollidingObject& obj1, const CollidingObject& obj2);
} // namespace imstk