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

#include "imstkRenderMaterial.h"
#include "imstkLogger.h"

namespace imstk
{
RenderMaterial::RenderMaterial()
{
    // Instantiating one type of each texture per material
    for (int i = 0; i < static_cast<int>(Texture::Type::None); i++)
    {
        m_textures.emplace_back(std::make_shared<Texture>("", static_cast<Texture::Type>(i)));
    }
}

void
RenderMaterial::setDisplayMode(const DisplayMode displayMode)
{
    if (displayMode != m_displayMode)
    {
        m_displayMode = displayMode;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setTessellated(const bool tessellated)
{
    if (tessellated != m_tessellated)
    {
        m_tessellated = tessellated;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setLineWidth(const float width)
{
    if (width != m_lineWidth)
    {
        m_lineWidth = width;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setPointSize(const float size)
{
    if (size != m_pointSize)
    {
        m_pointSize = size;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setBackFaceCulling(const bool culling)
{
    if (culling != m_backfaceCulling)
    {
        m_backfaceCulling = culling;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::backfaceCullingOn()
{
    this->setBackFaceCulling(true);
}

void
RenderMaterial::backfaceCullingOff()
{
    this->setBackFaceCulling(false);
}

void
RenderMaterial::setDiffuseColor(const Color& color)
{
    if (m_diffuseColor != color)
    {
        m_diffuseColor = color;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setColor(const Color& color)
{
    this->setDiffuseColor(color);
}

void
RenderMaterial::setSpecularColor(const Color& color)
{
    if (m_specularColor != color)
    {
        m_specularColor = color;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setAmbientColor(const Color& color)
{
    if (m_ambientColor != color)
    {
        m_ambientColor = color;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setMetalness(const float metalness)
{
    if (m_metalness != metalness)
    {
        m_metalness = metalness;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setRoughness(const float roughness)
{
    if (roughness != m_roughness)
    {
        m_roughness = roughness;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setEmissivity(const float emissivity)
{
    if (m_emissivity != emissivity)
    {
        m_emissivity = emissivity;
        postEvent(Event(EventType::Modified));
    }
}

std::shared_ptr<Texture>
RenderMaterial::getTexture(Texture::Type type)
{
    if (type >= Texture::Type::None)
    {
        LOG(WARNING) << "RenderMaterial::getTexture error: Invalid texture format";
        return nullptr;
    }
    return m_textures[(unsigned int)type];
}

void
RenderMaterial::addTexture(std::shared_ptr<Texture> texture)
{
    if (texture->getType() >= Texture::Type::None)
    {
        LOG(WARNING) << "RenderMaterial::addTexture: Invalid texture format";
        return;
    }
    m_textures[(unsigned int)texture->getType()] = texture;
    postEvent(Event(EventType::Modified));
}

void
RenderMaterial::setReceivesShadows(const bool receivesShadows)
{
    if (m_receivesShadows != receivesShadows)
    {
        m_receivesShadows = receivesShadows;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setCastsShadows(const bool castsShadows)
{
    if (m_castsShadows != castsShadows)
    {
        m_castsShadows = castsShadows;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setBlendMode(const RenderMaterial::BlendMode blendMode)
{
    if (m_blendMode != blendMode)
    {
        m_blendMode = blendMode;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setShadingModel(const ShadingModel& model)
{
    if (model != m_shadingModel)
    {
        m_shadingModel = model;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setOcclusionStrength(const float occlusionStrength)
{
    if (occlusionStrength != m_occlusionStrength)
    {
        m_occlusionStrength = occlusionStrength;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setNormalnStrength(const float normalStrength)
{
    if (normalStrength != m_normalStrength)
    {
        m_normalStrength = normalStrength;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setEdgeColor(const Color& color)
{
    if (color != m_edgeColor)
    {
        m_edgeColor = color;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setVertexColor(const Color& color)
{
    if (color != m_vertexColor)
    {
        m_vertexColor = color;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setOpacity(const float opacity)
{
    if (m_opacity != opacity)
    {
        m_opacity = opacity;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setBackfaceCulling(const bool culling)
{
    if (m_backfaceCulling != culling)
    {
        m_backfaceCulling = culling;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setColorLookupTable(std::shared_ptr<ColorFunction> lut)
{
    if (m_lookupTable != lut)
    {
        m_lookupTable = lut;
        postEvent(Event(EventType::Modified));
    }
}

void
RenderMaterial::setScalarVisibility(const bool scalarVisibility)
{
    if (m_scalarVisibility != scalarVisibility)
    {
        m_scalarVisibility = scalarVisibility;
        postEvent(Event(EventType::Modified));
    }
}
}
