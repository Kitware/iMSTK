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

#include "imstkGeometryMap.h"
#include "imstkLogger.h"

namespace imstk
{
void
GeometryMap::mute()
{
    m_isActive = false;
}

void
GeometryMap::activate()
{
    m_isActive = true;
}

void
GeometryMap::print() const
{
    LOG(INFO) << this->getTypeName();
}

bool
GeometryMap::isActive() const
{
    return m_isActive;
}

void
GeometryMap::setParentGeometry(std::shared_ptr<Geometry> parent)
{
    m_parentGeom = parent;
}

std::shared_ptr<Geometry>
GeometryMap::getParentGeometry() const
{
    return m_parentGeom;
}

void
GeometryMap::setChildGeometry(std::shared_ptr<Geometry> child)
{
    m_childGeom = child;
}

std::shared_ptr<Geometry>
GeometryMap::getChildGeometry() const
{
    return m_childGeom;
}

void
GeometryMap::initialize()
{
    CHECK(this->isValid()) << "Map is invalid!";

    this->compute();
}
} // imstk
