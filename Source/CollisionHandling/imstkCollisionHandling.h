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

#pragma once

#include "imstkCollisionData.h"

#include <memory>
#include <vector>

namespace imstk
{
class CollidingObject;
class InteractionPair;
class TaskNode;

///
/// \class CollisionHandling
///
/// \brief Base class for all collision handling classes
/// \todo: Abstract both GeometryAlgorithm and this together
///
class CollisionHandling
{
protected:
    CollisionHandling();

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

    std::shared_ptr<TaskNode> getTaskNode() const { return m_taskNode; }

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
        const CDElementVector<CollisionElement>& elementsA,
        const CDElementVector<CollisionElement>& elementsB) = 0;

protected:
    std::shared_ptr<CollidingObject> m_inputObjectA;
    std::shared_ptr<CollidingObject> m_inputObjectB;

    std::shared_ptr<const CollisionData> m_colData = nullptr; ///< Collision data
    std::shared_ptr<TaskNode> m_taskNode = nullptr;
};
}