/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <memory>
#include <string>

namespace imstk
{
class Texture;

///
/// \class IblProbe
///
/// \brief Image-based lighting probe
///
/// Image-based lighting (Ibl) probes are used to provide global illumination
/// using special cubemaps. The cubemaps are prefiltered and evaluated using a
/// lookup table (LUT) texture. The cubemaps should be preintegrated using
/// split-sum approximation.
///
class IblProbe
{
public:
    ///
    /// Constructor
    ///
    /// \param irradianceCubemapPath path to .dds irradiance (diffuse) cubemap
    /// \param radianceCubemapPath path to .dds radiance (specular) cubemap
    /// \param brdfLUTPath path to BRDF LUT (shouldn't be .dds)
    ///
    IblProbe(std::string irradianceCubemapPath,
             std::string radianceCubemapPath, std::string brdfLUTPath);

    ///
    /// \brief TODO
    ///
    std::shared_ptr<Texture> getIrradianceCubemapTexture();

    ///
    /// \brief TODO
    ///
    std::shared_ptr<Texture> getRadianceCubemapTexture();

    ///
    /// \brief TODO
    ///
    std::shared_ptr<Texture> getBrdfLUTTexture();

protected:
    std::string m_irradianceCubemapPath;
    std::string m_radianceCubemapPath;
    std::string m_brdfLUTPath;

    std::shared_ptr<Texture> m_irradianceCubemapTexture = nullptr;
    std::shared_ptr<Texture> m_radianceCubemapTexture   = nullptr;
    std::shared_ptr<Texture> m_brdfLUTTexture = nullptr;
};
} // namespace imstk
