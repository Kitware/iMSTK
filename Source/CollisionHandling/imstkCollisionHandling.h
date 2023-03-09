/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionData.h"
#include "imstkMacros.h"

namespace imstk
{
class Entity;
class InteractionPair;

///
/// \class CollisionHandling
///
/// \brief Base class for all collision handling classes
/// \todo: Abstract both GeometryAlgorithm and this together
///
class CollisionHandling
{
public:
    virtual ~CollisionHandling() = default;

    virtual const std::string getTypeName() const = 0;

    ///
    /// \brief Initialize and pre-fetch all required resources before the start of
    /// the simulation loop.
    ///
    virtual bool initialize() = 0;

    ///
    /// \brief Set/Get the input collision data used for handling
    ///
    void setInputCollisionData(std::shared_ptr<CollisionData> collisionData) { m_colData = collisionData; }
    std::shared_ptr<const CollisionData> getInputCollisionData() const { return m_colData; }

    ///
    /// \brief Handle the input collision data
    ///
    void update();

protected:
    CollisionHandling() = default;
    ///
    /// \brief Get the geometry used for handling
    /// defaults to the collision geometry
    ///
    virtual std::shared_ptr<Geometry> getCollidingGeometryA() = 0;
    virtual std::shared_ptr<Geometry> getCollidingGeometryB() = 0;

    ///
    /// \brief Handle the input collision data. Elements will be flipped
    /// (if needed) such that elementsA corresponds with inputObjectA and B with inputObjectB
    /// in the case CD is backwards from CH
    /// \param CD elements to resolve geomA
    /// \param CD elements to resolve geomB
    ///
    virtual void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) = 0;

    std::shared_ptr<const CollisionData> m_colData = nullptr; ///< Collision data
};
} // namespace imstk