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
class SphObject;

///
/// \class SphObjectCollision
///
/// \brief This class defines where collision should happen between SphObject
/// and Entity pipelines
///
class SphObjectCollision : public CollisionInteraction
{
public:
    SphObjectCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                       std::string cdType = "");
    ~SphObjectCollision() override = default;

    IMSTK_TYPE_NAME(SphObjectCollision)

    bool initialize() override;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
};
} // namespace imstk