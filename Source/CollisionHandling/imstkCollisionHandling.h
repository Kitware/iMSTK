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
    void setInputCollisionData(std::shared_ptr<CollisionData> collisionData);

    ///
    /// \brief Set/Get the input collision data used for handling
    /// This supports collating information from multiple collision detect algorithms
    ///
    void setInputCollisionData(std::shared_ptr<std::vector<std::shared_ptr<CollisionData>>> collisionVectorData);

    std::shared_ptr<const CollisionData> getInputCollisionData() const { return m_colData; }

    ///
    /// \brief Handle the input collision data
    ///

	inline void update()
	{
		m_updateFunction();
	}

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

    std::shared_ptr<CollidingObject> m_inputObjectA;
    std::shared_ptr<CollidingObject> m_inputObjectB;

    std::shared_ptr<const CollisionData> m_colData;             ///< Collision data

    /// Expansion to allow collision detection to return multiple types of collision data
    std::shared_ptr<std::vector<std::shared_ptr<CollisionData>>> m_colVectorData;

    // This is the function that is executed in \sa update()
    std::function<void()> m_updateFunction = []() {};

    // Checks the ordering of the elements and calls the virtual \sa handle() function
    void updateCollisionData(std::shared_ptr<const CollisionData> data);

    // Used to enable multiple /sa handle calls, m_clearData is true on the first pass
    // m_postConstraints is true on the last pass, if there is only one pass both will
    // be true
    bool m_clearData       = true;
    bool m_processConstraints = true;
};
} // namespace imstk