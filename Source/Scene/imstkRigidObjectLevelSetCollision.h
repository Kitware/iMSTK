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
class Collider;
class LevelSetModel;
class PbdMethod;
class PbdSystem;
class PointSet;
class TaskGraph;
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
    IMSTK_TYPE_NAME(RigidObjectLevelSetCollision) RigidObjectLevelSetCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2);
    ~RigidObjectLevelSetCollision() override = default;

    ///
    /// \brief Initialize the interaction based on set input before the simulation starts.
    ///
    bool initialize() override;

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

    struct
    {
        std::shared_ptr<PbdMethod> method;
        std::shared_ptr<Collider> collider;
        std::shared_ptr<PbdSystem> system;
        std::shared_ptr<TaskGraph> taskGraph;
        std::shared_ptr<PointSet> physicsGeometry;
        std::shared_ptr<VecDataArray<double, 3>> displacements;
    } m_objectA;

    struct
    {
        // \todo: To be deleted after LevelSetDeformableObject has been removed:
        std::shared_ptr<LevelSetDeformableObject> obj;

        std::shared_ptr<LevelSetModel> method;
        std::shared_ptr<Collider> collider;
        std::shared_ptr<TaskGraph> taskGraph;
    } m_objectB;
};
} // namespace imstk