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

#include "imstkObjectInteractionPair.h"

namespace imstk
{
class CollisionData;
class CollisionDetectionAlgorithm;
class CollisionHandling;
class CollidingObject;

///
/// \class CollisionPair
///
/// \brief CollisionPair is a specialization of InteractionPair that adds Handler functions.
/// The handler functions precede the interaction node/step as their own computational node/step.
/// The handler functions may be a singular handler node (that handles both AB concurrently) or
/// two separate concurrent nodes.
///
class CollisionPair : public ObjectInteractionPair
{
public:
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB);

    ///
    /// \brief Specifies a CollisionPair with two handles (one or both can be nullptr)
    ///
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB,
                  std::shared_ptr<CollisionDetectionAlgorithm> cd,
                  std::shared_ptr<CollisionHandling> chA,
                  std::shared_ptr<CollisionHandling> chB);

    ///
    /// \brief Specifies CollisionPair with an AB handler
    ///
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB,
                  std::shared_ptr<CollisionDetectionAlgorithm> cd,
                  std::shared_ptr<CollisionHandling> chAB);

    virtual ~CollisionPair() override = default;

public:
    /// \brief TODO
    void setCollisionDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect);
    void setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA);
    void setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB);
    void setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB);

    /// \brief TODO
    std::shared_ptr<CollisionDetectionAlgorithm> getCollisionDetection() const { return m_colDetect; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingA() const { return m_colHandlingA; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingB() const { return m_colHandlingB; }

    std::shared_ptr<TaskNode> getCollisionDetectionNode() const { return m_collisionDetectionNode; }
    std::shared_ptr<TaskNode> getCollisionHandlingANode() const { return m_collisionHandleANode; }
    std::shared_ptr<TaskNode> getCollisionHandlingBNode() const { return m_collisionHandleBNode; }

    void updateCollisionGeometry();

public:
    virtual void apply() override;

protected:
    std::shared_ptr<CollisionDetectionAlgorithm> m_colDetect = nullptr;    ///< Collision detection algorithm
    std::shared_ptr<CollisionHandling> m_colHandlingA = nullptr;
    std::shared_ptr<CollisionHandling> m_colHandlingB = nullptr;

    std::shared_ptr<TaskNode> m_collisionDetectionNode      = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleANode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleBNode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionGeometryUpdateNode = nullptr;
};
}
