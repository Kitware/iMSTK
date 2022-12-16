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
class RigidObject2;
class Entity;
template<typename T, int N> class VecDataArray;

///
/// \class RigidObjectObjectCollision
///
/// \brief This class defines a collision interaction between two RigidObjects
/// This involves a RigidObjCH which will generate 2 way or 1 way constraints for the RigidBodyModel/s
/// depending on which system they belong too
///
class RigidObjectCollision : public CollisionInteraction
{
public:
    RigidObjectCollision(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<Entity> obj2, std::string cdType = "");
    ~RigidObjectCollision() override = default;

    IMSTK_TYPE_NAME(RigidObjectCollision)

    void setBaumgarteStabilization(double stiffness);
    const double getBeta() const;

    void setFriction(double frictionalCoefficient);
    const double getFriction() const;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    void copyVertsToPrevious();

    void measureDisplacementFromPrevious();

    std::shared_ptr<VecDataArray<double, 3>> m_prevVertices;

protected:
    std::shared_ptr<TaskNode> m_copyVertToPrevNode      = nullptr;
    std::shared_ptr<TaskNode> m_computeDisplacementNode = nullptr;
};
} // namespace imstk