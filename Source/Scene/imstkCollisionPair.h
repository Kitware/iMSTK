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

#include "imstkCollisionDetection.h"
#include "imstkCollisionHandling.h"
#include "imstkObjectInteractionPair.h"

namespace imstk
{
struct CollisionData;
class CollidingObject;

///
/// \class CollisionPair
///
/// \brief CollisionPair is a specialization of InteractionPair that adds Handler functions.
/// The handler functions precede the interaction node/step as their own computational node/step.
/// The handler functions may be a singular handler node or two separate concurrent nodes.
///
class CollisionPair : public ObjectInteractionPair
{
public:
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB);

    ///
    /// \brief Specifies a CollisionPair with two handles (one or both can be nullptr)
    ///
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB,
                  std::shared_ptr<CollisionDetection> cd,
                  std::shared_ptr<CollisionHandling> chA,
                  std::shared_ptr<CollisionHandling> chB);

    ///
    /// \brief Specifies CollisionPair with an AB handler
    ///
    CollisionPair(std::shared_ptr<CollidingObject> objA, std::shared_ptr<CollidingObject> objB,
                  std::shared_ptr<CollisionDetection> cd,
                  std::shared_ptr<CollisionHandling> chAB);

    ~CollisionPair() = default;

public:
    /// \brief TODO
    void setCollisionDetection(std::shared_ptr<CollisionDetection> colDetect);
    void setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA);
    void setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB);
    void setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB);

    /// \brief TODO
    std::shared_ptr<CollisionDetection> getCollisionDetection() const { return m_colDetect; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingA() const { return m_colHandlingA; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingB() const { return m_colHandlingB; }

    /// \brief TODO
    std::shared_ptr<ComputeNode> getCollisionDetectionNode() const { return m_collisionDetectionNode; }
    std::shared_ptr<ComputeNode> getCollisionHandlingANode() const { return m_collisionHandleANode; }
    std::shared_ptr<ComputeNode> getCollisionHandlingBNode() const { return m_collisionHandleBNode; }

public:
    virtual void modifyComputeGraph() override;

protected:
    std::shared_ptr<CollisionDetection> m_colDetect    = nullptr; ///< Collision detection algorithm
    std::shared_ptr<CollisionData>      m_colData      = nullptr; ///< Common Collision Data
    std::shared_ptr<CollisionHandling>  m_colHandlingA = nullptr;
    std::shared_ptr<CollisionHandling>  m_colHandlingB = nullptr;

    std::shared_ptr<ComputeNode> m_collisionDetectionNode = nullptr;
    std::shared_ptr<ComputeNode> m_collisionHandleANode   = nullptr;
    std::shared_ptr<ComputeNode> m_collisionHandleBNode   = nullptr;
};
}
