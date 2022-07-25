/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObject.h"

namespace imstk
{
class CollisionData;
class CollisionDetectionAlgorithm;
class CollisionHandling;
class CollidingObject;

///
/// \class CollisionInteraction
///
/// \brief Abstract class for defining collision interactions between objects
///
class CollisionInteraction : public SceneObject
{
public:
    ~CollisionInteraction() override = default;

protected:
    CollisionInteraction(std::string objName,
                         std::shared_ptr<CollidingObject> obj1, std::shared_ptr<CollidingObject> obj2);

public:
    void setCollisionDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect);

    ///
    /// \brief Set the Collision Handling for object A
    ///
    void setCollisionHandlingA(std::shared_ptr<CollisionHandling> colHandlingA);

    ///
    /// \brief Set the Collision Handling for object B
    ///
    void setCollisionHandlingB(std::shared_ptr<CollisionHandling> colHandlingB);

    ///
    /// \brief Set the two-way Collision Handling for both objects
    ///
    void setCollisionHandlingAB(std::shared_ptr<CollisionHandling> colHandlingAB);

    std::shared_ptr<CollisionDetectionAlgorithm> getCollisionDetection() const { return m_colDetect; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingA() const { return m_colHandlingA; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingB() const { return m_colHandlingB; }
    std::shared_ptr<CollisionHandling> getCollisionHandlingAB() const { return m_colHandlingA; }

    std::shared_ptr<TaskNode> getCollisionDetectionNode() const { return m_collisionDetectionNode; }
    std::shared_ptr<TaskNode> getCollisionHandlingANode() const { return m_collisionHandleANode; }
    std::shared_ptr<TaskNode> getCollisionHandlingBNode() const { return m_collisionHandleBNode; }

    void updateCollisionGeometry();

    ///
    /// \brief Enable or disable the interaction, when disabled the interaction isn't executed and no response will happen
    ///@{
    virtual void setEnabled(const bool enabled);
    virtual bool getEnabled() const;
///@}

protected:
    ///
    /// \brief Update collision
    ///
    void updateCD();

    ///
    /// \brief Update handler A
    ///
    void updateCHA();

    ///
    /// \brief Update handler B
    ///
    void updateCHB();

    std::shared_ptr<CollidingObject> m_objA = nullptr;
    std::shared_ptr<CollidingObject> m_objB = nullptr;

    std::shared_ptr<CollisionDetectionAlgorithm> m_colDetect = nullptr; ///< Collision detection algorithm
    std::shared_ptr<CollisionHandling> m_colHandlingA = nullptr;
    std::shared_ptr<CollisionHandling> m_colHandlingB = nullptr;

    std::shared_ptr<TaskNode> m_collisionDetectionNode      = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleANode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionHandleBNode        = nullptr;
    std::shared_ptr<TaskNode> m_collisionGeometryUpdateNode = nullptr;
};
} // namespace imstk