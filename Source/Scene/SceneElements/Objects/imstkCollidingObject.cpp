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

#include "imstkCollidingObject.h"

namespace imstk
{
const std::shared_ptr<Geometry>&
CollidingObject::getCollidingGeometry() const
{
    return m_collidingGeometry;
}

void
CollidingObject::setCollidingGeometry(const std::shared_ptr<Geometry>& geometry)
{
    m_collidingGeometry = geometry;
}

std::shared_ptr<Geometry>
CollidingObject::getMasterGeometry() const
{
    return m_collidingGeometry;
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

const Vec3d&
CollidingObject::getForce() const
{
    return m_force;
}

void
CollidingObject::setForce(Vec3d force)
{
    m_force = force;
}

void
CollidingObject::resetForce()
{
    m_force.setConstant(0.0);
}

void
CollidingObject::appendForce(Vec3d force)
{
    m_force += force;
}

void
CollidingObject::updateGeometries()
{
    if (m_collidingToVisualMap)
    {
        m_collidingToVisualMap->apply();
    }
}
} // imstk
