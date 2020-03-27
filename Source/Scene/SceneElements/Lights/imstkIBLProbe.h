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

#pragma once

#include <string>

#include "imstkTexture.h"

namespace imstk
{
///
/// \class IBLProbe
///
/// \brief Image-based lighting probe
///
/// Image-based lighting (IBL) probes are used to provide global illumination
/// using special cubemaps. The cubemaps are prefiltered and evaluated using a
/// lookup table (LUT) texture. The cubemaps should be preintegrated using
/// split-sum approximation.
///
class IBLProbe
{
public:
    ///
    /// Constructor
    ///
    /// \param irradianceCubemapPath path to .dds irradiance (diffuse) cubemap
    /// \param radianceCubemapPath path to .dds radiance (specular) cubemap
    /// \param brdfLUTPath path to BRDF LUT (shouldn't be .dds)
    ///
    IBLProbe::IBLProbe(std::string irradianceCubemapPath,
                       std::string radianceCubemapPath,
                       std::string brdfLUTPath) :
        m_irradianceCubemapPath(irradianceCubemapPath),
        m_radianceCubemapPath(radianceCubemapPath),
        m_brdfLUTPath(brdfLUTPath),
        m_irradianceCubemapTexture(std::make_shared<Texture>(irradianceCubemapPath,
                           Texture::Type::IRRADIANCE_CUBEMAP)),
        m_radianceCubemapTexture(std::make_shared<Texture>(radianceCubemapPath,
                           Texture::Type::RADIANCE_CUBEMAP)),
        m_brdfLUTTexture(std::make_shared<Texture>(brdfLUTPath, Texture::Type::BRDF_LUT)) {}

    std::shared_ptr<Texture> getIrradianceCubemapTexture();
    std::shared_ptr<Texture> getRadianceCubemapTexture();
    std::shared_ptr<Texture> getBrdfLUTTexture();

protected:
    std::string m_irradianceCubemapPath;
    std::string m_radianceCubemapPath;
    std::string m_brdfLUTPath;

    std::shared_ptr<Texture> m_irradianceCubemapTexture = nullptr;
    std::shared_ptr<Texture> m_radianceCubemapTexture   = nullptr;
    std::shared_ptr<Texture> m_brdfLUTTexture = nullptr;
};
}
