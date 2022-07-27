/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkIBLProbe.h"
#include "imstkTexture.h"

namespace imstk
{
IblProbe::IblProbe(std::string irradianceCubemapPath,
                   std::string radianceCubemapPath, std::string brdfLUTPath) :
    m_irradianceCubemapPath(irradianceCubemapPath),
    m_radianceCubemapPath(radianceCubemapPath),
    m_brdfLUTPath(brdfLUTPath),
    m_irradianceCubemapTexture(std::make_shared<Texture>(irradianceCubemapPath,
            Texture::Type::IrradianceCubeMap)),
    m_radianceCubemapTexture(std::make_shared<Texture>(radianceCubemapPath,
            Texture::Type::RadianceCubeMap)),
    m_brdfLUTTexture(std::make_shared<Texture>(brdfLUTPath, Texture::Type::BRDF_LUT))
{
}

std::shared_ptr<Texture>
IblProbe::getIrradianceCubemapTexture()
{
    return m_irradianceCubemapTexture;
}

std::shared_ptr<Texture>
IblProbe::getRadianceCubemapTexture()
{
    return m_radianceCubemapTexture;
}

std::shared_ptr<Texture>
IblProbe::getBrdfLUTTexture()
{
    return m_brdfLUTTexture;
}
} // namespace imstk