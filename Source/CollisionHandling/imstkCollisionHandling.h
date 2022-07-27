/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionData.h"

namespace imstk
{
class CollidingObject;
class InteractionPair;

///
/// \class CollisionHandling
///
/// \brief Base class for all collision handling classes
/// \todo: Abstract both GeometryAlgorithm and this together
///
class CollisionHandling
{
protected:
    CollisionHandling() = default;

public:
    virtual ~CollisionHandling() = default;

    virtual const std::string getTypeName() const = 0;

public:
    ///
    /// \brief Set the input objects
    ///
    void setInputObjectA(std::shared_ptr<CollidingObject> objectA) { m_inputObjectA = objectA; }
    void setInputObjectB(std::shared_ptr<CollidingObject> objectB) { m_inputObjectB = objectB; }

    ///
    /// \brief Get the input objects
    ///
    std::shared_ptr<CollidingObject> getInputObjectA() const { return m_inputObjectA; }
    std::shared_ptr<CollidingObject> getInputObjectB() const { return m_inputObjectB; }

    ///
    /// \brief Get the geometry used for handling
    /// defaults to the collision geometry
    ///
    virtual std::shared_ptr<Geometry> getHandlingGeometryA();
    virtual std::shared_ptr<Geometry> getHandlingGeometryB();

    ///
    /// \brief Set/Get the input collision data used for handling
    ///
    void setInputCollisionData(std::shared_ptr<CollisionData> collisionData) { m_colData = collisionData; }
    std::shared_ptr<const CollisionData> getInputCollisionData() const { return m_colData; }

public:
    ///
    /// \brief Handle the input collision data
    ///
    void update();

protected:
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

protected:
    std::shared_ptr<CollidingObject> m_inputObjectA;
    std::shared_ptr<CollidingObject> m_inputObjectB;

    std::shared_ptr<const CollisionData> m_colData = nullptr; ///< Collision data
};
} // namespace imstk