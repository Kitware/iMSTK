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
    virtual ~RigidObjectLevelSetCollision() override = default;

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