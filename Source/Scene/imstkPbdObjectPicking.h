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

namespace imstk
{
class CollidingObject;
class PbdObject;

///
/// \class PbdObjectPicking
///
/// \brief This class defines a picking interaction between a PbdObject and
/// a CollidingObject with AnalyticalGeometry. It does picking via vertex
/// selection. The vertices are treated as infinite mass particles.
///
class PbdObjectPicking : public CollisionInteraction
{
public:
    PbdObjectPicking(std::shared_ptr<PbdObject> obj1, std::shared_ptr<CollidingObject> obj2, std::string cdType);
    virtual ~PbdObjectPicking() override = default;

    virtual const std::string getTypeName() const override { return "PbdObjectPicking"; }

    ///
    /// \brief Remove all picking nodes and constraints
    ///
    void endPick();

    ///
    /// \brief Add picking nodes nodes and constraints
    ///
    void beginPick();

    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<TaskNode> getPickingNode() const { return m_pickingNode; }

protected:
    std::shared_ptr<TaskNode> m_pickingNode = nullptr;
};
}