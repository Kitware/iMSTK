/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
        postModified();
    }
}

void
RenderMaterial::setLineWidth(const double width)
{
    if (width != m_lineWidth)
    {
        m_lineWidth = width;
        postModified();
    }
}

void
RenderMaterial::setPointSize(const double size)
{
    if (size != m_pointSize)
    {
        m_pointSize = size;
        postModified();
    }
}

void
RenderMaterial::setBackFaceCulling(const bool culling)
{
    if (culling != m_backfaceCulling)
    {
        m_backfaceCulling = culling;
        postModified();
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
        postModified();
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
        postModified();
    }
}

void
RenderMaterial::setAmbientColor(const Color& color)
{
    if (m_ambientColor != color)
    {
        m_ambientColor = color;
        postModified();
    }
}

void
RenderMaterial::setMetalness(const double metalness)
{
    if (m_metalness != metalness)
    {
        m_metalness = metalness;
        postModified();
    }
}

void
RenderMaterial::setRoughness(const double roughness)
{
    if (roughness != m_roughness)
    {
        m_roughness = roughness;
        postModified();
    }
}

void
RenderMaterial::setEmissivity(const double emissivity)
{
    if (m_emissivity != emissivity)
    {
        m_emissivity = emissivity;
        postModified();
    }
}

void
RenderMaterial::setAnisotropy(const double anisotropy)
{
    if (m_anisotropy != anisotropy)
    {
        m_anisotropy = anisotropy;
        postModified();
    }
}

void
RenderMaterial::setAnisotropyRotation(const double anisotropyRotation)
{
    if (m_anisotropyRotation != anisotropyRotation)
    {
        m_anisotropyRotation = anisotropyRotation;
        postModified();
    }
}

void
RenderMaterial::setBaseIOR(const double baseIOR)
{
    if (m_baseIOR != baseIOR)
    {
        m_baseIOR = baseIOR;
        postModified();
    }
}

void
RenderMaterial::setCoatColor(const Color& coatColor)
{
    if (m_coatColor != coatColor)
    {
        m_coatColor = coatColor;
        postModified();
    }
}

void
RenderMaterial::setCoatIOR(const double coatIOR)
{
    if (m_coatIOR != coatIOR)
    {
        m_coatIOR = coatIOR;
        postModified();
    }
}

void
RenderMaterial::setCoatNormalScale(const double coatNormalScale)
{
    if (m_coatNormalScale != coatNormalScale)
    {
        m_coatNormalScale = coatNormalScale;
        postModified();
    }
}

void
RenderMaterial::setCoatRoughness(const double coatRoughness)
{
    if (m_coatRoughness != coatRoughness)
    {
        m_coatRoughness = coatRoughness;
        postModified();
    }
}

void
RenderMaterial::setCoatStrength(const double coatStrength)
{
    if (m_coatStrength != coatStrength)
    {
        m_coatStrength = coatStrength;
        postModified();
    }
}

void
RenderMaterial::setEdgeTint(const Color& edgeTint)
{
    if (m_edgeTint != edgeTint)
    {
        m_edgeTint = edgeTint;
        postModified();
    }
}

std::shared_ptr<Texture>
RenderMaterial::getTexture(Texture::Type type)
{
    if (type >= Texture::Type::None)
    {
        LOG(WARNING) << "error: Invalid texture format";
        return nullptr;
    }
    return m_textures[(unsigned int)type];
}

void
RenderMaterial::addTexture(std::shared_ptr<Texture> texture)
{
    if (texture->getType() >= Texture::Type::None)
    {
        LOG(WARNING) << "Invalid texture format";
        return;
    }
    m_textures[static_cast<size_t>(texture->getType())] = texture;
    postEvent(Event(texturesModified()));
}

void
RenderMaterial::removeTexture(std::shared_ptr<Texture> texture)
{
    // The texture (object) must exist
    auto iter = std::find(m_textures.begin(), m_textures.end(), texture);
    if (iter != m_textures.end())
    {
        const size_t type = static_cast<size_t>(texture->getType());
        m_textures[type] = std::make_shared<Texture>("", static_cast<Texture::Type>(type));
        postEvent(Event(texturesModified()));
    }
}

void
RenderMaterial::removeTexture(Texture::Type type)
{
    // If the texture already has path "" then it is empty
    const size_t             typeInt = static_cast<size_t>(type);
    std::shared_ptr<Texture> prevTex = m_textures[typeInt];
    if (prevTex->getPath() != "")
    {
        m_textures[typeInt] = std::make_shared<Texture>("", type);
        postEvent(Event(texturesModified()));
    }
}

void
RenderMaterial::setBlendMode(const RenderMaterial::BlendMode blendMode)
{
    if (m_blendMode != blendMode)
    {
        m_blendMode = blendMode;
        postModified();
    }
}

void
RenderMaterial::setShadingModel(const ShadingModel& model)
{
    if (model != m_shadingModel)
    {
        m_shadingModel = model;
        postModified();
    }
}

void
RenderMaterial::setOcclusionStrength(const double occlusionStrength)
{
    if (occlusionStrength != m_occlusionStrength)
    {
        m_occlusionStrength = occlusionStrength;
        postModified();
    }
}

void
RenderMaterial::setNormalStrength(const double normalStrength)
{
    if (normalStrength != m_normalStrength)
    {
        m_normalStrength = normalStrength;
        postModified();
    }
}

void
RenderMaterial::setEdgeColor(const Color& color)
{
    if (color != m_edgeColor)
    {
        m_edgeColor = color;
        postModified();
    }
}

void
RenderMaterial::setVertexColor(const Color& color)
{
    if (color != m_vertexColor)
    {
        m_vertexColor = color;
        postModified();
    }
}

void
RenderMaterial::setOpacity(const double opacity)
{
    if (m_opacity != opacity)
    {
        m_opacity = opacity;
        postModified();
    }
}

void
RenderMaterial::setBackfaceCulling(const bool culling)
{
    if (m_backfaceCulling != culling)
    {
        m_backfaceCulling = culling;
        postModified();
    }
}

void
RenderMaterial::setColorLookupTable(std::shared_ptr<ColorFunction> lut)
{
    if (m_lookupTable != lut)
    {
        m_lookupTable = lut;
        postModified();
    }
}

void
RenderMaterial::setScalarVisibility(const bool scalarVisibility)
{
    if (m_scalarVisibility != scalarVisibility)
    {
        m_scalarVisibility = scalarVisibility;
        postModified();
    }
}
} // namespace imstk
