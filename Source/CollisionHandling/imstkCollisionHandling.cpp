/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionHandling.h"
#include "imstkCollider.h"
#include "imstkEntity.h"

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

    std::shared_ptr<Geometry> handleGeomA = getCollidingGeometryA();
    std::shared_ptr<Geometry> handleGeomB = getCollidingGeometryB();

    bool flipSides = false;
    // If the geometry of the CD exists on the input object B then flip
    if (m_colData->geomA != nullptr
        && m_colData->geomA == handleGeomB)
    {
        flipSides = true;
    }
    if (m_colData->geomB != nullptr
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
} // namespace imstk