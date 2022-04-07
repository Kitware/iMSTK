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

#include "imstkPbdObjectGrasping.h"

namespace imstk
{
class PbdObject;
class RigidObject2;
class TaskNode;

class PbdRigidObjectGrasping : public PbdObjectGrasping
{
protected:
    std::shared_ptr<RigidObject2> m_rbdObj = nullptr;
    std::shared_ptr<PbdObject>    m_pbdObj = nullptr;

public:
    PbdRigidObjectGrasping(
        std::shared_ptr<PbdObject>    obj1,
        std::shared_ptr<RigidObject2> obj2);

    ~PbdRigidObjectGrasping() override = default;
    IMSTK_TYPE_NAME(PbdRigidObjectGrasping)

    void updatePicking();

    void addConstraint(
        const std::vector<VertexMassPair>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<VertexMassPair>& ptsB,
        const std::vector<double>& weightsB,
        double stiffnessA, double stiffnessB)
    override;

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
};
} // namespace imstk