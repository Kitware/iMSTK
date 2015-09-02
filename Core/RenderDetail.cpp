// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Core/RenderDetail.h"

void RenderDetail::addShader(std::shared_ptr<UnifiedId> p_shaderID)
{
    shaderEnable.push_back(true);
    shaders.push_back(p_shaderID);
}
void RenderDetail::addVAO(std::shared_ptr<UnifiedId> p_shaderID)
{
    VAOs.push_back(p_shaderID);
    VAOEnable.push_back(true);
}

RenderDetail::RenderDetail()
{
    this->reset();
}

RenderDetail::RenderDetail(unsigned int type)
{
    this->reset();
    normalLength = 1.0;
    this->renderType = type;
}

void RenderDetail::reset()
{
    renderType = 0;
    highLightColor.rgba[0] = 1.0f;
    highLightColor.rgba[1] = 0.0f;
    highLightColor.rgba[2] = 0.0f;
    pointSize = 1;
    lineSize = 1;
    shadowColor.rgba[0] = 0.0f;
    shadowColor.rgba[1] = 0.0f;
    shadowColor.rgba[2] = 0.0f;
    shadowColor.rgba[3] = 0.5f;
    colorDiffuse = Color::colorWhite;
    colorAmbient = Color::colorWhite;
    colorSpecular = Color::colorWhite;
    normalColor = Color::colorGreen;
    wireFrameColor = Color::colorBlue;
    shininess = 50.0;
    debugDraw = false;
    castShadow = true;
    canGetShadow = true;
    textureFilename = "";
}

void RenderDetail::setNormalLength(const float len)
{
    this->normalLength = len;
}
void RenderDetail::setLineSize(const float size)
{
    this->lineSize = size;
}
void RenderDetail::setPointSize(const float size)
{
    this->pointSize = size;
}
void RenderDetail::setVertexColor(const Color vertColor)
{
    this->vertexRenderColor = vertColor;
}
void RenderDetail::setNormalColor(const Color normColor)
{
    this->normalColor = normColor;
}
void RenderDetail::setDiffuseColor(const Color diffColor)
{
    this->colorDiffuse = diffColor;
}
void RenderDetail::setAmbientColor(const Color ambColor)
{
    this->colorAmbient = ambColor;
}
void RenderDetail::setSpecularColor(const Color specColor)
{
    this->colorSpecular = specColor;
}
void RenderDetail::setShadowColor(const Color shadColor)
{
    this->shadowColor = shadColor;
}
void RenderDetail::setWireframeColor(const Color wireColor)
{
    this->wireFrameColor = wireColor;
}
void RenderDetail::setShininess(const float s)
{
    this->shininess = s;
}
void RenderDetail::setHighlightColor(const Color highlightColor)
{
    this->highLightColor = highlightColor;
}
const Color& RenderDetail::getColorDiffuse() const
{
    return colorDiffuse;
}
const Color& RenderDetail::getColorAmbient() const
{
    return colorAmbient;
}
const Color& RenderDetail::getColorSpecular() const
{
    return colorSpecular;
}
const float& RenderDetail::getShininess() const
{
    return shininess;
}
const unsigned int& RenderDetail::getRenderType() const
{
    return renderType;
}
const float& RenderDetail::getPointSize() const
{
    return pointSize;
}
const float& RenderDetail::getLineSize() const
{
    return lineSize;
}
const Color& RenderDetail::getNormalColor() const
{
    return this->normalColor;
}
const Color& RenderDetail::getHighLightColor() const
{
    return this->highLightColor;
}
const Color& RenderDetail::getVertexColor() const
{
    return this->vertexRenderColor;
}
const Color& RenderDetail::getShadowColor() const
{
    return this->shadowColor;
}
const bool& RenderDetail::getCastShadow() const
{
    return this->castShadow;
}
const bool& RenderDetail::getCanGetShadow() const
{
    return this->canGetShadow;
}
const Color& RenderDetail::getWireFrameColor() const
{
    return this->wireFrameColor;
}
const bool& RenderDetail::getDebugDraw() const
{
    return this->debugDraw;
}
const std::vector< std::shared_ptr<UnifiedId> >& RenderDetail::getShaders() const
{
    return this->shaders;
}
const std::vector< bool >& RenderDetail::getShaderEnable() const
{
    return this->shaderEnable;
}
const std::vector< std::shared_ptr<UnifiedId> >& RenderDetail::getVAOs() const
{
    return this->VAOs;
}
const std::vector< bool >& RenderDetail::getVAOEnable() const
{
    return this->VAOEnable;
}
void RenderDetail::setTextureFilename(const std::string& filename)
{
    this->textureFilename = filename;
}
const std::string &RenderDetail::getTextureFilename() const
{
    return this->textureFilename;
}
bool RenderDetail::renderTexture() const
{
    return !this->textureFilename.empty() && this->renderType & SIMMEDTK_RENDER_TEXTURE;
}
bool RenderDetail::renderNormals() const
{
    return  this->renderType & SIMMEDTK_RENDER_NORMALS;
}
