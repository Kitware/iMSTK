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
    // Check for nullptr in colliding objects
    if (&obj1 == nullptr || &obj2 == nullptr)
    {
        LOG(FATAL) << "Nullptr passed into getCDType";
        return std::string("");
    }

    std::string cdType = CDObjectFactory::getCDType(*obj1.getCollidingGeometry(), *obj2.getCollidingGeometry());
    return cdType;
}
} // namespace imstk