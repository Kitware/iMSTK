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

namespace imstk
{
RenderMaterial::RenderMaterial()
{
    // Instantiating one type of each texture per material
    for (int i = 0; i < (int)Texture::Type::NONE; i++)
    {
        m_textures.emplace_back(std::make_shared<Texture>("", (Texture::Type)i));
    }
}

const RenderMaterial::DisplayMode
RenderMaterial::getDisplayMode() const
{
    return m_displayMode;
}

void
RenderMaterial::setDisplayMode(const DisplayMode displayMode)
{
    if (displayMode == m_displayMode)
    {
        return;
    }
    m_displayMode = displayMode;
    m_stateModified = true;
    m_modified = true;
}

const float
RenderMaterial::getLineWidth() const
{
    return m_lineWidth;
}

void
RenderMaterial::setLineWidth(const float width)
{
    if (width == m_lineWidth)
    {
        return;
    }
    m_lineWidth = width;
    m_stateModified = true;
    m_modified = true;
}

const float
RenderMaterial::getPointSize() const
{
    return m_pointSize;
}

void
RenderMaterial::setPointSize(const float size)
{
    if (size == m_pointSize)
    {
        return;
    }
    m_pointSize = size;
    m_stateModified = true;
    m_modified = true;
}

const bool
RenderMaterial::getBackFaceCulling() const
{
    return m_backfaceCulling;
}

void
RenderMaterial::setBackFaceCulling(const bool culling)
{
    if (culling == m_backfaceCulling)
    {
        return;
    }
    m_backfaceCulling = culling;
    m_stateModified = true;
    m_modified = true;
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

const Color&
RenderMaterial::getDiffuseColor() const
{
    return m_diffuseColor;
}

void
RenderMaterial::setDiffuseColor(const Color color)
{
    m_diffuseColor = color;
    m_modified = true;
}

const Color&
RenderMaterial::getSpecularColor() const
{
    return m_specularColor;
}

void
RenderMaterial::setSpecularColor(const Color color)
{
    m_specularColor = color;
    m_modified = true;
}

const float&
RenderMaterial::getSpecularity() const
{
    return m_specularity;
}

void
RenderMaterial::setSpecularity(const float specularity)
{
    m_specularity = specularity;
}

std::shared_ptr<Texture>
RenderMaterial::getTexture(Texture::Type type)
{
    if (type >= Texture::Type::NONE)
    {
        LOG(WARNING) << "RenderMaterial::getTexture error: Invalid texture format";
        return nullptr;
    }
    return m_textures[type];
}

void
RenderMaterial::addTexture(std::shared_ptr<Texture> texture)
{
    if (texture->getType() >= Texture::Type::NONE)
    {
        LOG(WARNING) << "RenderMaterial::addTexture: Invalid texture format";
        return;
    }
    m_textures[texture->getType()] = texture;
}

void
RenderMaterial::flatShadingOn()
{
    m_flatShading = true;
}

void
RenderMaterial::flatShadingOff()
{
    m_flatShading = false;
}

void
RenderMaterial::setVisibilityOff()
{
    m_isVisible = false;
    m_modified = true;
}

void
RenderMaterial::setVisibilityOn()
{
    m_isVisible = true;
    m_modified = true;
}

bool
RenderMaterial::getVisibility() const
{
    return m_isVisible;
}

}
