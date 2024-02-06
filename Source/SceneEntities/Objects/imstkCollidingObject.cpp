/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollidingObject.h"
#include "imstkCDObjectFactory.h"
#include "imstkGeometry.h"
#include "imstkGeometryMap.h"

namespace imstk
{
bool
CollidingObject::initialize()
{
    if (!SceneObject::initialize())
    {
        return false;
    }

    if (m_collidingToVisualMap)
    {
        m_collidingToVisualMap->compute();
    }

    return true;
}

void
CollidingObject::clearCollisions()
{
    for (auto& item : m_collisions)
    {
        item.second.clear();
    }
}

void
CollidingObject::addCollision(std::shared_ptr<CollidingObject> other, std::shared_ptr<CollisionData> data)
{
    m_collisions[other].push_back(data);
}

const std::vector<std::shared_ptr<imstk::CollisionData>>&
CollidingObject::getCollisions(std::shared_ptr<CollidingObject> other) const
{
    static const std::vector<std::shared_ptr<imstk::CollisionData>> empty;

    if (m_collisions.find(other) != m_collisions.end())
    {
        return m_collisions.at(other);
    }
    else
    {
        return empty;
    }
}

void
CollidingObject::update()
{
    SceneObject::update();
    clearCollisions();
}

bool
CollidingObject::didCollide(std::shared_ptr<CollidingObject> other)
{
    return !m_collisions[other].empty();
}

std::shared_ptr<Geometry>
CollidingObject::getCollidingGeometry() const
{
    return m_collidingGeometry;
}

void
CollidingObject::setCollidingGeometry(std::shared_ptr<Geometry> geometry)
{
    m_collidingGeometry = geometry;
}

std::shared_ptr<GeometryMap>
CollidingObject::getCollidingToVisualMap() const
{
    return m_collidingToVisualMap;
}

void
CollidingObject::setCollidingToVisualMap(std::shared_ptr<GeometryMap> map)
{
    m_collidingToVisualMap = map;
}

void
CollidingObject::updateGeometries()
{
    if (m_collidingToVisualMap)
    {
        m_collidingToVisualMap->update();
        m_collidingToVisualMap->getChildGeometry()->postModified();
    }
    SceneObject::updateGeometries();
}

std::string
getCDType(const CollidingObject& obj1, const CollidingObject& obj2)
{
    std::string cdType = CDObjectFactory::getCDType(*obj1.getCollidingGeometry(), *obj2.getCollidingGeometry());
    return cdType;
}

void
CollidingObject::postModifiedAll()
{
    if (m_collidingGeometry != nullptr)
    {
        m_collidingGeometry->postModified();
    }
    SceneObject::postModifiedAll();
}
} // namespace imstk