/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkMacros.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_HIDES_CLASS_MEMBER

#include "imstkStVKForceModel.h"

#include <StVKElementABCDLoader.h>
#include <tetMesh.h>

DISABLE_WARNING_POP

namespace imstk
{
StvkForceModel::StvkForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                               const bool withGravity, const double gravity) : InternalForceModel()
{
    auto                   tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);
    vega::StVKElementABCD* precomputedIntegrals = vega::StVKElementABCDLoader::load(tetMesh.get());
    m_stVKInternalForces      = std::make_shared<vega::StVKInternalForces>(tetMesh.get(), precomputedIntegrals, withGravity, gravity);
    m_vegaStVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());
}
} // namespace imstk