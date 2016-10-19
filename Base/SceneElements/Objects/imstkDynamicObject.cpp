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

#include "imstkDynamicObject.h"

namespace imstk
{

std::shared_ptr<Geometry>
DynamicObject::getPhysicsGeometry() const
{
    return m_physicsGeometry;
}

void
DynamicObject::setPhysicsGeometry(std::shared_ptr<Geometry> geometry)
{
    m_physicsGeometry = geometry;
}

std::shared_ptr<Geometry>
DynamicObject::getMasterGeometry() const
{
    return m_physicsGeometry;
}

std::shared_ptr<GeometryMap>
DynamicObject::getPhysicsToCollidingMap() const
{
    return m_physicsToCollidingGeomMap;
}

void
DynamicObject::setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map)
{
    m_physicsToCollidingGeomMap = map;
}

std::shared_ptr<GeometryMap>
DynamicObject::getPhysicsToVisualMap() const
{
    return m_physicsToVisualGeomMap;
}

void
DynamicObject::setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map)
{
    m_physicsToVisualGeomMap = map;
}

std::shared_ptr<DynamicalModel>
DynamicObject::getDynamicalModel() const
{
    return m_dynamicalModel;
}

void
DynamicObject::setDynamicalModel(std::shared_ptr<DynamicalModel> dynaModel)
{
    m_dynamicalModel = dynaModel;
}

size_t
DynamicObject::getNumOfDOF() const
{
    return numDOF;
}

} // imstk
