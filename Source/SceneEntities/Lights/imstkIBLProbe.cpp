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