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
/// and CollidingObject pipelines
///
class SphObjectCollision : public CollisionInteraction
{
public:
    SphObjectCollision(std::shared_ptr<SphObject> obj1, std::shared_ptr<CollidingObject> obj2,
                       std::string cdType = "ImplicitGeometryToPointSetCD");
    ~SphObjectCollision() override = default;

    IMSTK_TYPE_NAME(SphObjectCollision)

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;
};
} // namespace imstk