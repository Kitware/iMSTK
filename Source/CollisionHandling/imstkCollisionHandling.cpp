/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollisionHandling.h"
#include "imstkCollidingObject.h"

namespace imstk
{
void
CollisionHandling::updateCollisionData(std::shared_ptr<const CollisionData> data)
{
    // Get the geometry and elements
    const std::vector<CollisionElement>* a = &data->elementsA;
    const std::vector<CollisionElement>* b = &data->elementsB;

    std::shared_ptr<Geometry> handleGeomA = getHandlingGeometryA();
    std::shared_ptr<Geometry> handleGeomB = getHandlingGeometryB();

    bool flipSides = false;
    // If the geometry of the CD exists on the input object B then flip
    if (data->geomA != nullptr
        && m_inputObjectB != nullptr
        && handleGeomB != nullptr
        && data->geomA == handleGeomB)
    {
        flipSides = true;
    }
    if (data->geomB != nullptr
        && m_inputObjectA != nullptr
        && handleGeomA != nullptr
        && data->geomB == handleGeomA)
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

void
CollisionHandling::setInputCollisionData(std::shared_ptr<CollisionData> collisionData)
{
    m_colData        = collisionData;
    m_colVectorData  = nullptr;
    m_updateFunction = [this]() {
                           if (m_colData == nullptr)
                           {
                               return;
                           }
                           m_clearData = true;
                           m_processConstraints = true;
                           updateCollisionData(m_colData);
                       };
}

void
CollisionHandling::setInputCollisionData(std::shared_ptr<std::vector<std::shared_ptr<CollisionData>>> collisionVectorData)
{
    m_colVectorData = collisionVectorData;
    m_colData       = nullptr;

    m_updateFunction = [this]() {
                           if (m_colVectorData == nullptr)
                           {
                               return;
                           }
                           m_clearData = true;
                           m_processConstraints = false;
                           for (size_t i = 0; i < m_colVectorData->size(); ++i)
                           {
                               // CCD accesses this ...
                               m_colData = m_colVectorData->at(i);
                               m_processConstraints = i == m_colVectorData->size() - 1;
                               updateCollisionData(m_colData);
                               m_clearData = false;
                           }
                           m_colData = nullptr;
                       };
}
} // namespace imstk