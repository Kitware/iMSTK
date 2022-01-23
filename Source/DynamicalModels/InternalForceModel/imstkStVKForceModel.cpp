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