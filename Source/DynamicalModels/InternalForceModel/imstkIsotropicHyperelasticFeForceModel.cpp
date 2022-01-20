/*= ========================================================================

Library: iMSTK

Copyright(c) Kitware, Inc.& Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.

======================================================================== = */

#include "imstkIsotropicHyperelasticFeForceModel.h"
#include "imstkLogger.h"

#include <MooneyRivlinIsotropicMaterial.h>
#include <neoHookeanIsotropicMaterial.h>
#include <StVKIsotropicMaterial.h>

namespace imstk
{
IsotropicHyperelasticFeForceModel::IsotropicHyperelasticFeForceModel(const HyperElasticMaterialType materialType,
                                                                     std::shared_ptr<vega::VolumetricMesh> mesh,
                                                                     const double inversionThreshold, const bool withGravity, const double gravity) : InternalForceModel()
{
    auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);

    const int    enableCompressionResistance = 1;
    const double compressionResistance       = 500;
    switch (materialType)
    {
    case HyperElasticMaterialType::StVK:
        m_isotropicMaterial = std::make_shared<vega::StVKIsotropicMaterial>(
            tetMesh.get(),
            enableCompressionResistance,
            compressionResistance);
        break;

    case HyperElasticMaterialType::NeoHookean:
        m_isotropicMaterial = std::make_shared<vega::NeoHookeanIsotropicMaterial>(
            tetMesh.get(),
            enableCompressionResistance,
            compressionResistance);
        break;

    case HyperElasticMaterialType::MooneyRivlin:
        m_isotropicMaterial = std::make_shared<vega::MooneyRivlinIsotropicMaterial>(
            tetMesh.get(),
            enableCompressionResistance,
            compressionResistance);
        break;

    default:
        LOG(WARNING) << "Error: Invalid hyperelastic material type.";
        break;
    }

    m_isotropicHyperelasticFem = std::make_shared<vega::IsotropicHyperelasticFEM>(
        tetMesh.get(),
        m_isotropicMaterial.get(),
        inversionThreshold,
        withGravity,
        gravity);
}
}