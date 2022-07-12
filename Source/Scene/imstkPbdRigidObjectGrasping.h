/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdObjectGrasping.h"

namespace imstk
{
class PbdObject;
class RigidObject2;
class TaskNode;

class PbdRigidObjectGrasping : public PbdObjectGrasping
{
public:
    PbdRigidObjectGrasping(
        std::shared_ptr<PbdObject>    obj1,
        std::shared_ptr<RigidObject2> obj2);

    ~PbdRigidObjectGrasping() override = default;

    IMSTK_TYPE_NAME(PbdRigidObjectGrasping)

    void updatePicking();

    void addConstraint(
        const std::vector<PbdParticleId>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<PbdParticleId>& ptsB,
        const std::vector<double>& weightsB,
        double stiffnessA, double stiffnessB) override;

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<RigidObject2> m_rbdObj = nullptr;
    std::shared_ptr<PbdObject>    m_pbdObj = nullptr;
};
} // namespace imstk