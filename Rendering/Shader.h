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

#ifndef SMSHADER_H
#define SMSHADER_H

// STL includes
#include <unordered_map>
#include <list>
#include <string>
#include <vector>
#include <memory>

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ErrorLog.h"
#include "Rendering/GLUtils.h"
#include "Core/Timer.h"
#include "Core/BaseMesh.h"

namespace Core {
class BaseMesh;
}
class SurfaceMesh;

struct TextureShaderAssignment
{
    GLint textureShaderGLassignment; // the id that Shader creates...
    int textureId; // Id from texture manager
    std::string shaderParamName; // The parameters that shaders use
};

// \brief Base shader class. It provides loading, initializing, binding,
//  enabling disabling current shader functionality.Also it provides
//  frequent check of the shader code to make shader development easy.
class Shader: public CoreClass
{
public:
#ifdef SIMMEDTK_OPENGL_SHADER
    // \brief creates GLSL vertex shader
    void createVertexShaderGLSL();

    // \brief creates GLSL fragment shader
    void createFragmentShaderGLSL();

    // \brief creates GLSL geometry shader
    void createGeometryShaderGLSL();

    // \brief reloads the vertex shader
    void reloadVertexShaderGLSL();

    // \brief reloads the fragment shader
    void reloadFragmentShaderGLSL();

    // \brief reloads the geometry shader
    void reloadGeometryShaderGLSL();

    // \brief Retrieves a shader uniform value location and stores it
    //
    // \param p_paramName Name of the uniform value to find
    // \param p_shaderProgramObject Shader object to find the uniform in
    // \param p_shaderParamsString List of shader parameter names to append to
    // if the uniform variable is found
    // \param p_shaderParams List of shader parameter locations to append to
    // if the uniform variable is found
    GLint addShaderParamGLSL(const std::string& p_paramName,
                               const GLhandleARB p_shaderProgramObject,
                               std::vector<std::string>& p_shaderParamsString,
                               std::vector<GLint>& p_shaderParams);

    // \brief add vertex, fragment and geometry shaders
    GLint addVertexShaderParamGLSL(const std::string& p_paramVertex);

    GLint addFragmentShaderParamGLSL(const std::string& p_paramFragment);

    GLint addGeometryShaderParamGLSL(const std::string& p_paramGeometry);
    // \brief returns the program object
    GLuint getProgramObject()
    {
        return shaderProgramObject;
    }

    // \brief returns vertex shader object
    GLuint getVertexShaderObject()
    {
        return vertexShaderObject;
    }

    // \brief retursn fragment shader object
    GLuint getFragmentShaderObject()
    {
        return fragmentShaderObject;
    }

    // \brief returns geomtry shader object
    GLuint getGeometryShaderObject()
    {
        return geometryShaderObject;
    }

    // \brief Attaches The texture ID to the mesh
    void attachTexture(std::shared_ptr<UnifiedId> p_meshID, int p_textureID);

    // \brief assigns the texture by name if you don't know the textureID
    bool attachTexture(std::shared_ptr<UnifiedId> p_meshID, const std::string& p_textureName, const std::string& p_textureShaderName);

    void autoGetTextureIds();

#endif

    void getAttribAndParamLocations();

protected:
    // \brief Creates a shader object from shader content,
    // then adds the shader object to a program object for use
    //
    // \param p_shaderObject An empty shader handle to be assigned the value of
    // a newly created shader
    // \param p_shaderProgramObject A shader program to add the newly created
    // shader to
    // \param p_shaderContent A string containing the shader program
    // \param p_shaderType The type of shader, can be:
    // GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER,
    // GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER
    void createShaderGLSL(GLhandleARB &p_shaderObject,
                          const GLhandleARB p_shaderProgramObject,
                          const std::string& p_shaderContent,
                          GLenum p_shaderType);

    // \brief Reloads and recompiles the shader object
    //
    // \param p_shaderObject Shader handle to be reloaded
    // \param p_shaderContent A string containing the shader program
    void reloadShaderGLSL(const GLhandleARB p_shaderObject,
                          const std::string& p_shaderContent);

    // \brief reloads all shaders
    bool reLoadAllShaders();

    // \brief check opengl error
    bool checkGLError();

public:
    // \brief constructor gets the error log class
    Shader(std::shared_ptr<ErrorLog> logger);

    // \brief initialized the shaders.
    // \param vertexProgFileName   vertex program file name
    // \param fragmentProgFileName fragment program file name
    // \param geometryProgFileName geometry program file name
    bool initShaders(const std::string& p_vertexProgFileName,
                       const std::string& p_fragmentProgFileName,
                       const std::string& p_geometryProgFileName);

    // \brief  enables the shader
    void enableShader();

    // \brief  disables the shader
    void disableShader();

    // \brief Initialize the shader..This is called automatically.
    virtual void initDraw();

#ifdef SIMMEDTK_OPENGL_SHADER
    // \brief add parameter for Vertex Shader
    GLint addVertexShaderParam(const std::string& p_paramVertex);

    // \brief add parameter for Fragment Shader
    GLint addFragmentShaderParam(const std::string& p_paramFragment);

    // \brief add parameter for Geometry Shader
    GLint addGeometryShaderParam(const std::string& p_paramGeometry);

    // \brief add parameter for all Shaders
    GLint addShaderParamForAll(const std::string& p_paramName);

    // \brief attrib parameters for Shaders
    GLint addShaderParamAttrib(const std::string& p_paramName);

    void createTextureParam(const std::string& p_textureNameInShaderCode);

    bool setShaderFileName(const std::string& p_vertexFileName,
                             const std::string& p_geometryFileName,
                             const std::string& p_fragmentFileName);

    bool setModelViewMatrixShaderName(const std::string& p_modelviewMatrixName);

    bool setProjectionMatrixShaderName(const std::string& p_projectionName);

    void updateGLSLMatwithOPENGL();

    // \brief returns the shader attrrib param
    GLint getShaderParamForAll(const std::string& p_paramName) const;

    // \brief returns the fragment shader uniform param
    GLint getFragmentShaderParam(const std::string& p_paramName) const;

    // \brief returns the attrib location param
    GLint getShaderAtrribParam(const std::string& p_paramName) const;

    void createParam(const std::string& p_param);

    int createAttrib(const std::string& p_attrib);

    GLint queryUniformLocation(const std::string& p_param);

#endif

    // \brief checks whether the shader routine is updated
    bool checkShaderUpdate(int milliseconds);

    // \brief turn on/off the error checking
    void enableCheckingErrors(bool p_checkError);

    // \brief cleans up of the shader objects
    ~Shader();

    // \brief void implementations for virtual functions. needs to be overwritten for any specific uniform bindings
    virtual void predraw(std::shared_ptr<Core::BaseMesh>/*mesh*/){};

    virtual void predraw(std::shared_ptr<SurfaceMesh>/*mesh*/){};

    virtual void posdraw(std::shared_ptr<Core::BaseMesh>/*mesh*/){};

    virtual void posdraw(std::shared_ptr<SurfaceMesh>/*mesh*/){};

    static std::shared_ptr<Shader> getShader(std::shared_ptr<UnifiedId> p_shaderID);

    bool readShaderContent(const std::string& p_file, std::string& p_content);

    static void initGLShaders();

    void activeGLTextures(std::shared_ptr<UnifiedId> p_id);

    void activeGLVertAttribs(int p_id, core::Vec3d *p_vecs, int p_size);

    void registerShader();

    void print() const;

    // \brief restores the shader from the last saved one
    void restoreAndEnableCurrent();

    // \brief save the last active shader and disable it.
    void saveAndDisableCurrent();

    GLint getTangentAttributes()
    {
        return tangentAttrib;
    }

protected:
    std::shared_ptr<ErrorLog> log; //
    bool checkErrorEnabled; // if the error check is enabled or not. If it is checked, opengl errors are queried and if there is, they will be stored in logger
    GLint tangentAttrib;

private:
    static std::unordered_map<int, std::shared_ptr<Shader>> shaders;
    std::unordered_multimap<int, TextureShaderAssignment> texAssignments; //
    std::unordered_map<std::string, GLint> textureGLBind; // This stores the opengl binded texture id
    std::vector<std::string> vertexShaderParamsString; // stores the parameters for vertex shader
    std::vector<std::string> fragmentShaderParamsString; // stores the parameters for fragment shader
    std::vector<std::string> geometryShaderParamsString; // stores the parameters for geometry shader
    std::vector<std::string> attribParamsString; // stores the attribute parameters
    Timer time; // time for periodically checnking the shader

    GLint projectionMatrix; // holds the unitform location for projection matrix. That is needed in newer versions of GLSL
    GLint modelViewMatrix; // holds the unitform location for modelview matrix. That is needed in newer versions of GLSL

    std::string vertexProgFileName; //
    std::string fragmentProgFileName; //
    std::string geometryProgFileName; //
    std::string vertexShaderContent; //stores the content of the vertex shader file
    std::string fragmentShaderContent; // stores the content of the fragment  shader file
    std::string geometryShaderContent; // stores the content of the geometry shader file
    std::string modelViewMatrixName; //
    std::string projectionMatrixName; //

    bool vertexProgramExist; // if the vertex shader exists this will be true
    bool fragmentProgramExist; // if the fragment shader exists this will be true
    bool geometryProgramExist; // if the geometry shader exists this will be true
    bool currentShaderEnabled; // if the currentShader is enabled or not

    static std::shared_ptr<Shader> currentShader; // stores the current Active shader.
    static std::shared_ptr<Shader> savedShader; // It is also used to save and restore the current shader is disabled for a while to use

#ifdef SIMMEDTK_OPENGL_SHADER
    GLhandleARB vertexShaderObject; // vertex shader object
    GLhandleARB fragmentShaderObject; // fragment  shader object
    GLhandleARB geometryShaderObject; // fragment  shader object
    GLhandleARB shaderProgramObject; // shader program object.
    std::vector<GLint> vertexShaderParams; // stores the id of the parameters in vertex shader
    std::vector<GLint> fragmentShaderParams; // stores the id of the parameters in fragment shader
    std::vector<GLint> geometryShaderParams; // the id of the parameters in geometry shader
    std::vector<GLint> attribShaderParams; // the id of the parameters in geometry shader
#endif
};

#endif
