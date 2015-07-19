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
#include "RenderDetail.h"
//#include "Shader/Shader.h"

void smRenderDetail::addShader(std::shared_ptr<smUnifiedId> p_shaderID)
{

    shaderEnable.push_back(true);
    shaders.push_back(p_shaderID);
}
void smRenderDetail::addVAO(std::shared_ptr<smUnifiedId> p_shaderID)
{

    VAOs.push_back(p_shaderID);
    VAOEnable.push_back(true);
}

smRenderDetail::smRenderDetail()
{
  this->reset();
}

smRenderDetail::smRenderDetail(unsigned int type)
{
  this->reset();
  normalLength = 1.0;
  this->renderType = type;
}

void smRenderDetail::reset()
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
  colorDiffuse = smColor::colorWhite;
  colorAmbient = smColor::colorWhite;
  colorSpecular = smColor::colorWhite;
  normalColor = smColor::colorGreen;
  wireFrameColor = smColor::colorBlue;
  shininess = 50.0;
  debugDraw = false;
  castShadow = true;
  canGetShadow = true;
}

void smRenderDetail::setNormalLength(const float len)
{
    this->normalLength = len;
}
void smRenderDetail::setLineSize(const float size)
{
    this->lineSize = size;
}
void smRenderDetail::setPointSize(const float size)
{
    this->pointSize = size;
}
void smRenderDetail::setVertexColor(const smColor vertColor)
{
    this->vertexRenderColor = vertColor;
}
void smRenderDetail::setNormalColor(const smColor normColor)
{
    this->normalColor = normColor;
}
void smRenderDetail::setDiffuseColor(const smColor diffColor)
{
    this->colorDiffuse = diffColor;
}
void smRenderDetail::setAmbientColor(const smColor ambColor)
{
    this->colorAmbient = ambColor;
}
void smRenderDetail::setSpecularColor(const smColor specColor)
{
    this->colorSpecular = specColor;
}
void smRenderDetail::setShadowColor(const smColor shadColor)
{
    this->shadowColor = shadColor;
}
void smRenderDetail::setWireframeColor(const smColor wireColor)
{
    this->wireFrameColor = wireColor;
}
void smRenderDetail::setShininess(const float s)
{
    this->shininess = s;
}
void smRenderDetail::setHighlightColor(const smColor highlightColor)
{
    this->highLightColor = highlightColor;
}
const smColor& smRenderDetail::getColorDiffuse() const
{
    return colorDiffuse;
}
const smColor& smRenderDetail::getColorAmbient() const
{
    return colorAmbient;
}
const smColor& smRenderDetail::getColorSpecular() const
{
    return colorSpecular;
}
const float& smRenderDetail::getShininess() const
{
    return shininess;
}
const unsigned int& smRenderDetail::getRenderType() const
{
    return renderType;
}
const float& smRenderDetail::getPointSize() const
{
    return pointSize;
}
const float& smRenderDetail::getLineSize() const
{
    return lineSize;
}
const smColor& smRenderDetail::getNormalColor() const
{
    return this->normalColor;
}
const smColor& smRenderDetail::getHighLightColor() const
{
    return this->highLightColor;
}
const smColor& smRenderDetail::getVertexColor() const
{
    return this->vertexRenderColor;
}
const smColor& smRenderDetail::getShadowColor() const
{
    return this->shadowColor;
}
const bool& smRenderDetail::getCastShadow() const
{
    return this->castShadow;
}
const bool& smRenderDetail::getCanGetShadow() const
{
    return this->canGetShadow;
}
const smColor& smRenderDetail::getWireFrameColor() const
{
    return this->wireFrameColor;
}
const bool& smRenderDetail::getDebugDraw() const
{
    return this->debugDraw;
}
const std::vector< std::shared_ptr<smUnifiedId> >& smRenderDetail::getShaders() const
{
    return this->shaders;
}
const std::vector< bool >& smRenderDetail::getShaderEnable() const
{
    return this->shaderEnable;
}
const std::vector< std::shared_ptr<smUnifiedId> >& smRenderDetail::getVAOs() const
{
    return this->VAOs;
}
const std::vector< bool >& smRenderDetail::getVAOEnable() const
{
    return this->VAOEnable;
}
