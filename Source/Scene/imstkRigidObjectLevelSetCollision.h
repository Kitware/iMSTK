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
class LevelSetDeformableObject;
class RigidObject2;
template<typename T, int N> class VecDataArray;

///
/// \class RigidObjectLevelSetCollision
///
/// \brief This class defines a collision interaction pipeline between a
/// RigidObject and LevelSetDeformableObject.
///
class RigidObjectLevelSetCollision : public CollisionInteraction
{
public:
    RigidObjectLevelSetCollision(std::shared_ptr<RigidObject2> obj1, std::shared_ptr<LevelSetDeformableObject> obj2);
    ~RigidObjectLevelSetCollision() override = default;

    IMSTK_TYPE_NAME(RigidObjectLevelSetCollision)

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