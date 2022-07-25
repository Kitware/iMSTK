/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
} // namespace imstk