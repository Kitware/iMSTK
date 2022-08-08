/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"

namespace imstk
{
class PbdObject;
class RigidObject2;

///
/// \class PbdRigidObjectCollision
///
/// \brief This class defines a collision interaction between a first order PbdObject
/// and second order RigidObject2. This will induce response in both models.
///
class PbdRigidObjectCollision : public CollisionInteraction
{
public:
    ///
    /// \brief Constructor for PbdObject-PbdObject or PbdObject-CollidingObject collisions
    ///
    PbdRigidObjectCollision(std::shared_ptr<PbdObject> obj1, std::shared_ptr<RigidObject2> obj2,
                            std::string cdType = "");

    ~PbdRigidObjectCollision() override = default;

    IMSTK_TYPE_NAME(PbdRigidObjectCollision)

    void setRestitution(const double restitution);
    const double getRestitution() const;

    void setFriction(const double friction);
    const double getFriction() const;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
};
} // namespace imstk