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
#ifndef SMRENDERDETAIL_H
#define SMRENDERDETAIL_H

#include "Core/Config.h"
#include "Core/Color.h"
#include "Core/ConfigRendering.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

class VisualArtifact;
class CoreClass;
class Model;
struct UnifiedId;

///\brief Hold a pointer to a source of geometry that render details can use for drawing.
struct GeometrySource {
  CoreClass* sceneObject;
  VisualArtifact* analyticObject;
  Model *model;

  GeometrySource()
    : sceneObject(nullptr), analyticObject(nullptr)
    { }
  void setSource(CoreClass* src)
    {
    this->sceneObject = src;
    this->analyticObject = nullptr;
    this->model = nullptr;
    }
  void setSource(VisualArtifact* src)
    {
    this->sceneObject = nullptr;
    this->model = nullptr;
    this->analyticObject = src;
    }
  void setSource(Model* src)
    {
    this->sceneObject = nullptr;
    this->model = src;
    this->analyticObject = nullptr;
    }
  template<typename T>
  T* sourceAs() const
    {
    T* result;
    if ((result = dynamic_cast<T*>(sceneObject)))
    {
      return result;
    }
    if ((result = dynamic_cast<T*>(analyticObject)))
    {
      return result;
    }
    if ((result = dynamic_cast<T*>(model)))
    {
        return result;
    }
    return nullptr;
    }
};

/// \brief RenderDetail has rendering options and features.
///It shows how the mesh should be rendered
struct RenderDetail
{
public:
    typedef std::shared_ptr<RenderDetail> Ptr;

    RenderDetail();
    RenderDetail(unsigned int type);

    void reset();

    /// \brief attachment of shader
    void addShader(std::shared_ptr<UnifiedId> p_shaderID);

    /// \brief attachment of VAO
    void addVAO(std::shared_ptr<UnifiedId> p_shaderID);

    const Color &getColorDiffuse() const;

    const Color &getColorAmbient() const;

    const Color &getColorSpecular() const;

    const float &getShininess() const;

    const unsigned int &getRenderType() const;

    const float &getPointSize() const;

    const float &getLineSize() const;

    const Color &getNormalColor() const;

    const Color &getHighLightColor() const;

    const Color &getVertexColor() const;

    const Color &getShadowColor() const;

    const bool &getCastShadow() const;

    const bool &getCanGetShadow() const;

    const Color &getWireFrameColor() const;

    const bool &getDebugDraw() const;

    const std::vector<std::shared_ptr<UnifiedId>> &getShaders() const;

    const std::vector<bool> &getShaderEnable() const;

    const std::vector<std::shared_ptr<UnifiedId>> &getVAOs() const;

    const std::vector<bool> &getVAOEnable() const;

    void setPointSize(const float size);

    void setLineSize(const float size);

    void setVertexColor(const Color vertColor);

    void setHighlightColor(const Color highlightColor);

    void setNormalColor(const Color highlightColor);

    void setShininess(const float s);

    void setNormalLength(const float len);

    void setDiffuseColor(const Color diffColor);

    void setAmbientColor(const Color ambColor);

    void setSpecularColor(const Color specColor);

    void setShadowColor(const Color shadColor);

    void setWireframeColor(const Color wireColor);

    void setTextureFilename(const std::string &filename);
    const std::string &getTextureFilename() const;
    bool renderTexture() const;

    bool renderNormals() const;
    bool renderWireframe() const;
    bool renderFaces() const;

    const float &getOpacity() const;
    void setOpacity(const float &value);

    ///
    /// @brief Add a shader program to the the list of programs (vtk)
    /// @param program String containing the actual program
    ///
    void addShaderProgram(const std::string &program)
    {
        this->shaderPrograms.push_back(program);
    }

    std::vector<std::string> &getShaderPrograms()
    {
        return this->shaderPrograms;
    }

    ///
    /// @brief Add a shader program for partial replacement of vtk default shaders.
    /// @param type 0 = vertex, 1 = Fragment, 2 = Geometry
    /// @param program String containing the actual program
    ///
    void addShaderProgramReplacement(int type, const std::string &from,
                                     const std::string &to)
    {
        std::array<std::string,2> replacement = {from,to};
        this->shaderProgramReplacements[type].push_back(replacement);
    }

    const Color &getBackground() const
    {
        return this->background;
    }

    const Color &getBackgroundTop() const
    {
        return this->faceBackgroundTop;
    }

    const Color &getBackgroundBottom() const
    {
        return this->faceBackgroundBottom;
    }

    void setBackground(const Color &value)
    {
        this->background = value;
    }

    std::map<int,std::vector<std::array<std::string,2>>>
    &getShaderProgramReplacements()
    {
        return this->shaderProgramReplacements;
    }

    bool hasShaders()
    {
        return !this->shaderPrograms.empty() ||
               !this->shaderProgramReplacements.empty();
    }

public:
    unsigned int renderType; // render type
    Color colorDiffuse; // diffuse color
    Color colorAmbient; // ambient color
    Color colorSpecular; // specular color
    Color highLightColor; // highlight color
    Color vertexRenderColor; // vertex color
    Color shadowColor; // shadow color
    bool castShadow; // object can generate a shadow or not
    bool canGetShadow; // object can get the shadow or not
    Color normalColor; // normal color
    Color wireFrameColor; // wire frame color
    float pointSize; // point size if rendering of vertices are enabled
    float lineSize; // line width size
    float shininess; // specular shinness
    bool debugDraw; // debug draw enabled or not
    float normalLength; // length of rendered normals
    float opacity; // opacity
    std::string textureFilename; // file name for the texture attached]
    Color background;

    Color faceBackgroundBottom;
    Color faceBackgroundTop;

    std::vector<std::shared_ptr<UnifiedId>> shaders; // attached shaders
    std::vector<bool> shaderEnable; // enable/disable any attached shader
    std::vector<std::shared_ptr<UnifiedId>> VAOs; // stores  VAO IDs
    std::vector<bool> VAOEnable; // enable/disable any attached VAO
    std::vector<std::string> shaderPrograms;
    std::map<int,std::vector<std::array<std::string,2>>> shaderProgramReplacements;
};

#endif // SMRENDERDETAIL_H
