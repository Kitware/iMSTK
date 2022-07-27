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
class SphObject;

///
/// \class SphObjectCollision
///
/// \brief This class defines where collision should happen between SphObject
/// and CollidingObject pipelines
///
class SphObjectCollision : public CollisionInteraction
{
public:
    SphObjectCollision(std::shared_ptr<SphObject> obj1, std::shared_ptr<CollidingObject> obj2,
                       std::string cdType = "");
    ~SphObjectCollision() override = default;

    IMSTK_TYPE_NAME(SphObjectCollision)

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
};
} // namespace imstk