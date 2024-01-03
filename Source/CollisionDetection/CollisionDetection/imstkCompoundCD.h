/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"

namespace imstk
{
///
/// \class PointSetToPlaneCD
///
/// \brief PointSet to unidirectional plane collision detection
/// Generates point-direction contact data.
/// By default only generates contact data for the pointset.
///
class CompoundCD : public CollisionDetectionAlgorithm
{
public:
    CompoundCD();
    ~CompoundCD() override = default;

    IMSTK_TYPE_NAME(CompoundCD);

    void requestUpdate() override;

protected:

    bool areInputsValid() override;

private:
    std::vector<std::shared_ptr<CollisionDetectionAlgorithm>> m_cdAlgorithms;
};
} // namespace imstk
