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
#ifndef SMRENDERDETAIL_H
#define SMRENDERDETAIL_H

#include "Core/Config.h"
#include "Core/Color.h"
#include "Core/ConfigRendering.h"

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <array>

class VisualArtifact;
class CoreClass;
class Model;
struct UnifiedId;

class vtkOpenGLTexture;

///
/// \brief Hold a pointer to a source of geometry that render details can use for drawing.
///
struct GeometrySource
{
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
        if((result = dynamic_cast<T*>(sceneObject)))
        {
            return result;
        }
        if((result = dynamic_cast<T*>(analyticObject)))
        {
            return result;
        }
        if((result = dynamic_cast<T*>(model)))
        {
            return result;
        }
        return nullptr;
    }
};

struct TextureDetail
{
    std::string textureName;
    std::string fileName;
    std::string shaderBinding;
    std::string shaderProgramName;
    GLint shaderUniformGL;
    vtkOpenGLTexture* vtexture;
    static std::map<std::string, vtkOpenGLTexture*> textures;
};

struct ShaderDetail
{
    ShaderDetail()
    {
        initialized = false;
        geometryShaderExists = false;
    }
    std::string vertexShaderFileName;
    std::string vertexShaderSource;

    std::string fragmentShaderFileName;
    std::string fragmentShaderSource;

    bool geometryShaderExists;
    std::string geometryShaderFileName;
    std::string geometryShaderSource;
    std::string shaderProgramName;
    bool initialized;

    //std::string tessellationShaderFileName;
    //std::string tessellationShaderSource;
    //std::string computeShaderFileName;
    //std::string computeShaderSource;
};

class Shaders
{
public:
    Shaders();
    static bool createShader(std::string shaderProgramName, std::string vertexShaderFileName, std::string fragmentShaderFileName, std::string geometryShaderFileName);
    static std::map<std::string, ShaderDetail>  &getShaderPrograms();
    static bool getShaderProgram(std::string shaderProgramName, ShaderDetail &shaderDetail);
    static bool shaderExists(std::string shaderProgramName);

protected:
    static std::map<std::string, ShaderDetail> shaderPrograms;
};

/// \brief RenderDetail has rendering options and features.
///It shows how the mesh should be rendered
struct RenderDetail
{
public:
    typedef std::shared_ptr<RenderDetail> Ptr;

    ///
    /// \brief Constructors/Destructor
    ///
    RenderDetail();
    ~RenderDetail();
    RenderDetail(unsigned int type);

    ///
    /// @brief  Color setters
    ///
    const Color &getAmbientColor() const;
    const Color &getDiffuseColor() const;
    const Color &getSpecularColor() const;
    const Color &getHighLightColor() const;
    const Color &getNormalColor() const;
    const Color &getShadowColor() const;
    const Color &getVertexColor() const;
    const Color &getWireFrameColor() const;

    ///
    /// @brief  Color getters
    ///
    void setAmbientColor(const Color &color);
    void setDiffuseColor(const Color &color);
    void setHighlightColor(const Color &color);
    void setNormalColor(const Color &color);
    void setShadowColor(const Color &color);
    void setSpecularColor(const Color &color);
    void setVertexColor(const Color &color);
    void setWireframeColor(const Color &color);

    ///
    /// \brief Get/Set background color
    ///
    const Color &getBackground() const;
    void setBackground(const Color &value);
    const Color &getBackgroundTop() const
    {
        return this->faceBackgroundTop;
    }

    const Color &getBackgroundBottom() const
    {
        return this->faceBackgroundBottom;
    }

    ///
    /// \brief Get/Set opacity value
    ///
    const float &getOpacity() const;
    void setOpacity(const float &value);

    ///
    /// \brief Get/Set specular power
    ///
    const float &getShininess() const;
    void setShininess(const float &value);

    ///
    /// \brief Get/Set the length of the normal vector to draw
    ///
    const float &getNormalLength() const;
    void setNormalLength(const float &value);

    ///
    /// \brief Get/Set size of point to draw
    ///
    const float &getPointSize() const;
    void setPointSize(const float &value);

    ///
    /// \brief Get/Set size of line to draw
    ///
    const float &getLineSize() const;
    void setLineSize(const float &value);

    ///
    /// \brief Can the object get the shadow
    ///
    const bool &getCanGetShadow() const;

    ///
    /// \brief Can the object cast a shadow
    ///
    const bool &getCastShadow() const;

    ///
    /// \brief Debug mode
    ///
    const bool &getDebugDraw() const;

    ///
    /// \brief Render type
    ///
    const unsigned int &getRenderType() const;

    ///
    /// \brief Get enabled/disabled shaders array
    ///
    const std::vector<bool> &getShaderEnable() const;

    ///
    /// \brief Get VAO's array
    ///
    const std::vector<std::shared_ptr<UnifiedId>> &getVAOs() const;

    ///
    /// \brief Get enabled/disabled VAO array
    ///
    const std::vector<bool> &getVAOEnable() const;

    ///
    /// \brief Set/Get the binding texture filename
    ///
    void setTextureFilename(const std::string &filename);
    const std::string &getTextureFilename() const;

    ///
    /// \brief Set/Get the binding texture filename for a shader. shaderBinding is for texture name in the shader name
    ///
    void addTexture(const std::string &textureName, const std::string &filename,
                    const std::string &shaderBinding, const std::string &shaderProgramName);
    std::map<std::string, TextureDetail>& getTextures();
    int getNumberOfTextures();

    ///
    /// \brief Returns true if you want to draw texture map
    ///
    void setRenderTexture(bool value);
    bool renderTexture() const;

    ///
    /// \brief Returns true if you want to draw normalsmap
    ///
    void setRenderNormals(bool value);
    bool renderNormals() const;

    ///
    /// \brief Returns true if you want to draw wireframe
    ///
    void setRenderWireframe(bool value);
    bool renderWireframe() const;

    ///
    /// \brief Returns true if you want to draw faces
    ///
    void setRenderFaces(bool value);
    bool renderFaces() const;

    ///
    /// @brief Add a shader program to the the list (vtk)
    ///
    void addShaderProgram(int shaderType, const std::string &programFilename, const std::string & programName);
    void addShaderProgram(const std::string &shaderProgramName);
    std::string  getShaderProgram();

    ///
    /// \brief Return list of shader programs
    ///
    std::map<std::string, bool> &getShaderPrograms();

    ///
    /// @brief Add a shader program for partial replacement of vtk default shaders.
    /// @param type 0 = vertex, 1 = Fragment, 2 = Geometry
    /// @param program String containing the actual program
    ///
    void addShaderProgramReplacement(int type, const std::string &from,
                                     const std::string &to);


    ///
    /// \brief Return list of shader program replacements
    ///
    std::map<int, std::vector<std::array<std::string, 2>>>
    &getShaderProgramReplacements();

    ///
    /// \brief Retrurns true if there is vtk shader in the list
    ///
    bool hasShaders();

    ///
    /// \brief attachment of shader
    ///
    void addShader(std::shared_ptr<UnifiedId> p_shaderID);

    ///
    /// \brief Get shaders array
    ///
    const std::vector<std::shared_ptr<UnifiedId>> &getShaders() const;

    ///
    /// \brief attachment of VAO
    ///
    void addVAO(std::shared_ptr<UnifiedId> p_shaderID);

    ///
    /// \brief Reset variables and containers
    ///
    void reset();

    ///
    /// \brief Set render type
    ///
    void setRenderType(unsigned int p_renderType);

    ///
    /// @brief Add an attribute to a program to the the list (vtk)
    ///
    void addShaderAttribute(int shaderType, const std::string &attributeName);

private:
    bool canGetShadow; // object can get the shadow or not
    bool castShadow; // object can generate a shadow or not
    bool debugDraw; // debug draw enabled or not
    Color background; // backgroung color
    Color faceBackgroundBottom;
    Color faceBackgroundTop;
    Color ambientColor; // ambient color
    Color diffuseColor; // diffuse color
    Color specularColor; // specular color
    Color highLightColor; // highlight color
    Color normalColor; // normal color
    Color shadowColor; // shadow color
    Color vertexRenderColor; // vertex color
    Color wireFrameColor; // wire frame color
    float lineSize; // line width size
    float normalLength; // length of rendered normals
    float opacity; // opacity
    float pointSize; // point size if rendering of vertices are enabled
    float shininess; // specular shinness
    std::string textureFilename; // file name for the texture attached]
    unsigned int renderType; // render type

    std::string shaderProgramName;
    bool hasShader;

    std::map<std::string,bool> shaderPrograms;
    std::map<int, std::vector<std::array<std::string, 2>>> shaderProgramReplacements;
    std::vector<bool> shaderEnable; // enable/disable any attached shader
    std::vector<bool> VAOEnable; // enable/disable any attached VAO
    std::vector<std::shared_ptr<UnifiedId>> shaders; // attached shaders
    std::vector<std::shared_ptr<UnifiedId>> VAOs; // stores  VAO IDs
    std::map<std::string,int> shaderAttributes;
    std::map<std::string, TextureDetail> textures;
};

#endif // SMRENDERDETAIL_H
