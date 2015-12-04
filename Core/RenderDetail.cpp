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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/RenderDetail.h"
#include <fstream>
#include <sstream>
#include <iostream>

//---------------------------------------------------------------------------
// Shaders
//---------------------------------------------------------------------------
std::map<std::string, ShaderDetail> Shaders::shaderPrograms;

//---------------------------------------------------------------------------
bool Shaders::shaderExists(std::string shaderProgramName)
{
    std::map<std::string, ShaderDetail>::iterator it= shaderPrograms.find(shaderProgramName);
    if (it == shaderPrograms.end())
    {
      return false;
    }
    else
    {
      return true;
    }
}

//---------------------------------------------------------------------------
bool Shaders::getShaderProgram(std::string shaderProgramName, ShaderDetail &shaderDetail)
{
    std::map<std::string, ShaderDetail>::iterator it;
    it = shaderPrograms.find(shaderProgramName);
    if (it == shaderPrograms.end())
    {
      return false;
    }
    else
    {
      shaderDetail = shaderPrograms[shaderProgramName];
      return true;
    }
}

//---------------------------------------------------------------------------
bool Shaders::createShader(std::string shaderProgramName, std::string vertexShaderFileName,
                           std::string fragmentShaderFileName, std::string geometryShaderFileName)
{
    ShaderDetail shaderDetail;
    std::map<std::string, ShaderDetail> &shaderPrograms = Shaders::getShaderPrograms();

    std::ifstream shaderFileStream(vertexShaderFileName.c_str(), std::ifstream::in);
    if (!shaderFileStream.is_open())
    {
        std::cerr << "Error opening the shader program: " << vertexShaderFileName << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << shaderFileStream.rdbuf();

    shaderDetail.vertexShaderFileName = vertexShaderFileName;
    shaderDetail.vertexShaderSource = buffer.str();
    //std::cout << shaderDetail.vertexShaderSource << std::endl;

    //loading the fragment shader source code
    shaderFileStream.close();
    shaderFileStream.clear();
    buffer.str(std::string());
    buffer.clear();

    shaderFileStream.open(fragmentShaderFileName.c_str(), std::ifstream::in);
    if (!shaderFileStream.is_open())
    {
        std::cerr << "Error opening the shader program: " << fragmentShaderFileName << std::endl;
        return false;
    }

    buffer << shaderFileStream.rdbuf();
    shaderDetail.fragmentShaderFileName = fragmentShaderFileName;
    shaderDetail.fragmentShaderSource = buffer.str();
    //std::cout << shaderDetail.fragmentShaderSource << std::endl;

    shaderFileStream.close();
    shaderFileStream.clear();
    buffer.str(std::string());
    buffer.clear();

    if (!geometryShaderFileName.empty())
    {
        shaderFileStream.open(geometryShaderFileName.c_str(), std::ifstream::in);
        if (!shaderFileStream.is_open())
        {
            std::cerr << "Error opening the shader program: " << geometryShaderFileName << std::endl;
            return false;
        }
        buffer << shaderFileStream.rdbuf();
        shaderDetail.geometryShaderFileName = geometryShaderFileName;
        shaderDetail.geometryShaderSource = buffer.str();
        shaderDetail.geometryShaderExists = true;
        shaderFileStream.close();
        shaderFileStream.clear();
        buffer.str(std::string());
        buffer.clear();
    }
    shaderDetail.shaderProgramName = shaderProgramName;
    shaderPrograms.emplace(shaderProgramName, shaderDetail);

    return true;
}

//---------------------------------------------------------------------------
std::map<std::string, ShaderDetail>  &Shaders::getShaderPrograms()
{
    return Shaders::shaderPrograms;
}

//---------------------------------------------------------------------------
// RenderDetail
//---------------------------------------------------------------------------
RenderDetail::RenderDetail()
{
    this->reset();
}

//---------------------------------------------------------------------------
RenderDetail::RenderDetail(unsigned int type)
{
    this->reset();
    this->normalLength = 1.0;
    this->renderType = type;
    this->hasShader = false;
}

//---------------------------------------------------------------------------
RenderDetail::~RenderDetail() {}

//---------------------------------------------------------------------------
const Color& RenderDetail::getAmbientColor() const
{
    return this->ambientColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getDiffuseColor() const
{
    return this->diffuseColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getSpecularColor() const
{
    return this->specularColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getHighLightColor() const
{
    return this->highLightColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getNormalColor() const
{
    return this->normalColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getShadowColor() const
{
    return this->shadowColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getVertexColor() const
{
    return this->vertexRenderColor;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getWireFrameColor() const
{
    return this->wireFrameColor;
}

//---------------------------------------------------------------------------
void RenderDetail::setAmbientColor(const Color &color)
{
    this->ambientColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setDiffuseColor(const Color &color)
{
    this->diffuseColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setHighlightColor(const Color &color)
{
    this->highLightColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setNormalColor(const Color &color)
{
    this->normalColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setShadowColor(const Color &color)
{
    this->shadowColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setSpecularColor(const Color &color)
{
    this->specularColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setVertexColor(const Color &color)
{
    this->vertexRenderColor = color;
}

//---------------------------------------------------------------------------
void RenderDetail::setWireframeColor(const Color &color)
{
    this->wireFrameColor = color;
}

//---------------------------------------------------------------------------
const Color& RenderDetail::getBackground() const
{
    return this->background;
}

//---------------------------------------------------------------------------
void RenderDetail::setBackground(const Color &color)
{
    this->background = color;
}

//---------------------------------------------------------------------------
const float &RenderDetail::getOpacity() const
{
    return this->opacity;
}

//---------------------------------------------------------------------------
void RenderDetail::setOpacity(const float &opacityValue)
{
    this->opacity = opacityValue;
}

//---------------------------------------------------------------------------
const float& RenderDetail::getShininess() const
{
    return this->shininess;
}

//---------------------------------------------------------------------------
void RenderDetail::setShininess(const float &value)
{
    this->shininess = value;
}

//---------------------------------------------------------------------------
const float& RenderDetail::getNormalLength() const
{
    return this->normalLength;
}

//---------------------------------------------------------------------------
void RenderDetail::setNormalLength(const float &value)
{
    this->normalLength = value;
}

//---------------------------------------------------------------------------
const float& RenderDetail::getPointSize() const
{
    return this->pointSize;
}

//---------------------------------------------------------------------------
void RenderDetail::setPointSize(const float &value)
{
    this->pointSize = value;
}

//---------------------------------------------------------------------------
const float& RenderDetail::getLineSize() const
{
    return this->lineSize;
}

//---------------------------------------------------------------------------
void RenderDetail::setLineSize(const float &value)
{
    this->lineSize = value;
}

//---------------------------------------------------------------------------
const bool& RenderDetail::getCanGetShadow() const
{
    return this->canGetShadow;
}

//---------------------------------------------------------------------------
const bool& RenderDetail::getCastShadow() const
{
    return this->castShadow;
}

//---------------------------------------------------------------------------
const bool& RenderDetail::getDebugDraw() const
{
    return this->debugDraw;
}

//---------------------------------------------------------------------------
const unsigned int& RenderDetail::getRenderType() const
{
    return this->renderType;
}

//---------------------------------------------------------------------------
const std::vector< bool >& RenderDetail::getShaderEnable() const
{
    return this->shaderEnable;
}

//---------------------------------------------------------------------------
const std::vector< std::shared_ptr<UnifiedId> >& RenderDetail::getVAOs() const
{
    return this->VAOs;
}

//---------------------------------------------------------------------------
const std::vector< bool >& RenderDetail::getVAOEnable() const
{
    return this->VAOEnable;
}

//---------------------------------------------------------------------------
void RenderDetail::setRenderTexture(bool value)
{
    if(value)
    {
        this->renderType |= SIMMEDTK_RENDER_TEXTURE;
    }
    else
    {
        this->renderType |= ~SIMMEDTK_RENDER_TEXTURE;
    }
}

//---------------------------------------------------------------------------
bool RenderDetail::renderTexture() const
{
    return !this->textureFilename.empty() && (this->renderType & SIMMEDTK_RENDER_TEXTURE);
}

//---------------------------------------------------------------------------
void RenderDetail::setRenderNormals(bool value)
{
    if(value)
    {
        this->renderType |= SIMMEDTK_RENDER_NORMALS;
    }
    else
    {
        this->renderType |= ~SIMMEDTK_RENDER_NORMALS;
    }
}

//---------------------------------------------------------------------------
bool RenderDetail::renderNormals() const
{
    return  this->renderType & SIMMEDTK_RENDER_NORMALS;
}

//---------------------------------------------------------------------------
void RenderDetail::setRenderWireframe(bool value)
{
    if(value)
    {
        this->renderType |= SIMMEDTK_RENDER_WIREFRAME;
    }
    else
    {
        this->renderType |= ~SIMMEDTK_RENDER_WIREFRAME;
    }
}

//---------------------------------------------------------------------------
bool RenderDetail::renderWireframe() const
{
    return this->renderType & SIMMEDTK_RENDER_WIREFRAME;
}

//---------------------------------------------------------------------------
void RenderDetail::setRenderFaces(bool value)
{
    if(value)
    {
        this->renderType |= SIMMEDTK_RENDER_FACES;
    }
    else
    {
        this->renderType |= ~SIMMEDTK_RENDER_FACES;
    }
}

//---------------------------------------------------------------------------
bool RenderDetail::renderFaces() const
{
    return this->renderType & SIMMEDTK_RENDER_FACES;
}

//---------------------------------------------------------------------------
void RenderDetail::addShaderProgram(const std::string &shaderProgramName)
{
    if (Shaders::shaderExists(shaderProgramName))
    {
        this->hasShader = true;
        this->shaderProgramName.assign(shaderProgramName);
    }
}
//This will deprecate
void RenderDetail::addShaderProgram(int shaderType, const std::string& programFilename, const std::string &shaderProgramName)
{
    ShaderDetail shaderDetail;

    std::ifstream shaderFileStream(programFilename.c_str(), std::ifstream::in);
    if(!shaderFileStream.is_open())
    {
        std::cerr << "Error opening the shader program: " << programFilename << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << shaderFileStream.rdbuf();
    std::map<std::string, ShaderDetail> &shaderPrograms = Shaders::getShaderPrograms();
    if (!(shaderPrograms.find(shaderProgramName) == shaderPrograms.end())){
      shaderDetail = shaderPrograms[shaderProgramName];
    }
    if (shaderType == 0/*vtkShader::Vertex*/){//the numeric value will be changed with vtkShader::Vertex
      shaderDetail.vertexShaderFileName=programFilename;
      shaderDetail.vertexShaderSource=buffer.str();
    }
    if (shaderType == 1/*vtkShader::Fragment*/){//the numeric value will be changed with  vtkShader::Fragment
      //auto &shaderDetail = shaderPrograms[shaderProgramName];
      shaderDetail.fragmentShaderFileName=programFilename;
      shaderDetail.fragmentShaderSource=buffer.str();
    }
    shaderPrograms.emplace(shaderProgramName, shaderDetail);
}

//---------------------------------------------------------------------------
std::string  RenderDetail::getShaderProgram()
{
    return shaderProgramName;
}

//---------------------------------------------------------------------------
void RenderDetail::addShaderProgramReplacement(int type, const std::string& from, const std::string& to)
{
    std::array<std::string, 2> replacement = {from, to};
    this->shaderProgramReplacements[type].push_back(replacement);
}

//---------------------------------------------------------------------------
std::map< int, std::vector< std::array< std::string, int(2) > > >& RenderDetail::getShaderProgramReplacements()
{
    return this->shaderProgramReplacements;
}

//---------------------------------------------------------------------------
bool RenderDetail::hasShaders()
{
    return hasShader;
}

//---------------------------------------------------------------------------
void RenderDetail::addShader(std::shared_ptr<UnifiedId> p_shaderID)
{
    this->shaderEnable.push_back(true);
    this->shaders.push_back(p_shaderID);
}

//---------------------------------------------------------------------------
const std::vector< std::shared_ptr<UnifiedId> >& RenderDetail::getShaders() const
{
    return this->shaders;
}

//---------------------------------------------------------------------------
void RenderDetail::addVAO(std::shared_ptr<UnifiedId> p_shaderID)
{
    this->VAOs.push_back(p_shaderID);
    this->VAOEnable.push_back(true);
}

//---------------------------------------------------------------------------
void RenderDetail::reset()
{
    this->background.rgba[0] = 81.0/255.0;
    this->background.rgba[1] = 87.0/255.0;
    this->background.rgba[2] = 110.0/255.0;
    this->background.rgba[3] = 1.0;
    this->canGetShadow = true;
    this->castShadow = true;
    this->ambientColor = Color::colorGray;
    this->diffuseColor = Color::colorGray;
    this->specularColor = Color::colorWhite;
    this->debugDraw = false;
    this->highLightColor.rgba[0] = 1.0;
    this->highLightColor.rgba[1] = 0.0;
    this->highLightColor.rgba[2] = 0.0;
    this->lineSize = 1;
    this->normalColor = Color::colorGreen;
    this->opacity = 1.0;
    this->pointSize = 1;
    this->renderType = 0;
    this->shadowColor.rgba[0] = 0.0;
    this->shadowColor.rgba[1] = 0.0;
    this->shadowColor.rgba[2] = 0.0;
    this->shadowColor.rgba[3] = 0.5;
    this->shininess = 50.0;
    this->textureFilename = "";
    this->wireFrameColor = Color::colorBlue;
    this->faceBackgroundBottom.setValue(0.8, 0.8, 0.8, 1.0);
    this->faceBackgroundTop.setValue(0.45, 0.45, 0.8, 1.0);
}

//---------------------------------------------------------------------------
void RenderDetail::setRenderType(unsigned int p_renderType)
{
    renderType = p_renderType;
}

//---------------------------------------------------------------------------
void RenderDetail::addShaderAttribute(int shaderType, const std::string &attributeName)
{
    // TODO (Tansel)
    std::cerr<<"Error: RenderDetail::addShaderAttribute not yet implemented."<<std::endl;
}

//---------------------------------------------------------------------------
void RenderDetail::addTexture(const std::string &textureName, const std::string &filename,
                              const std::string &shaderBinding, const std::string &shaderProgramName)
{
    this->textures[textureName].textureName = textureName;
    this->textures[textureName].fileName = filename;
    this->textures[textureName].shaderBinding = shaderBinding;
    this->textures[textureName].shaderProgramName = shaderProgramName;
    this->textures[textureName].shaderUniformGL = -1; //not defined intially
}

//---------------------------------------------------------------------------
std::map<std::string, TextureDetail>  &RenderDetail::getTextures()
{
    return this->textures;
}

//---------------------------------------------------------------------------
int RenderDetail::getNumberOfTextures()
{
    return this->textures.size();
}

// static
std::map<std::string, vtkOpenGLTexture*> TextureDetail::textures;
