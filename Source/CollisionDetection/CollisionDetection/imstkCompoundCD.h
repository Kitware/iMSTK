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
/// \class CompoundCD
///
/// \brief Collision detection that supports a geometry consisting
///     of multiple subgeometries.
/// For the actual calcualation the information gets passed to the appropriate
/// shape/subshape CD algorithm. Currently Does not support adding/removing a shape during
/// runtime
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
    /// List of cd algorithms that are being used
    std::vector<std::shared_ptr<CollisionDetectionAlgorithm>> m_cdAlgorithms;
};
} // namespace imstk
