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

#include "imstkCollisionHandling.h"
#include "imstkCollidingObject.h"

namespace imstk
{
void
CollisionHandling::update()
{
    if (m_colData == nullptr)
    {
        return;
    }

    // Get the geometry and elements
    const std::vector<CollisionElement>* a = &m_colData->elementsA;
    const std::vector<CollisionElement>* b = &m_colData->elementsB;

    std::shared_ptr<Geometry> handleGeomA = getHandlingGeometryA();
    std::shared_ptr<Geometry> handleGeomB = getHandlingGeometryB();

    bool flipSides = false;
    // If the geometry of the CD exists on the input object B then flip
    if (m_colData->geomA != nullptr
        && m_inputObjectB != nullptr
        && handleGeomB != nullptr
        && m_colData->geomA == handleGeomB)
    {
        flipSides = true;
    }
    if (m_colData->geomB != nullptr
        && m_inputObjectA != nullptr
        && handleGeomA != nullptr
        && m_colData->geomB == handleGeomA)
    {
        flipSides = true;
    }

    if (flipSides)
    {
        std::swap(a, b);
    }

    handle(*a, *b);
}

std::shared_ptr<Geometry>
CollisionHandling::getHandlingGeometryA()
{
    return (m_inputObjectA == nullptr) ? nullptr : m_inputObjectA->getCollidingGeometry();
}

std::shared_ptr<Geometry>
CollisionHandling::getHandlingGeometryB()
{
    return (m_inputObjectB == nullptr) ? nullptr : m_inputObjectB->getCollidingGeometry();
}
}