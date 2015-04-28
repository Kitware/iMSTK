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

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smTimer.h"

class smMesh;
class smSurfaceMesh;

struct smTextureShaderAssignment
{
    /// \brief the id that smShader creates...
    smGLInt textureShaderGLassignment;
    /// \brief Id from texture manager
    smInt textureId;
    /// \brief The parameters that shaders use
    smString shaderParamName;
};

/// \brief this is the generic shader class. It provides loading, initializing, binding,
///enabling disabling current shader functionality.Also it provides frequent check of the shader code
///to make shader development easy.
class smShader: public smCoreClass
{
public:
    smGLInt tangentAttrib;
protected:
    static std::unordered_map<smInt, smShader *> shaders;

    /// \brief Error Loging
    smErrorLog *log;
    smString vertexProgFileName;
    smString fragmentProgFileName;
    smString geometryProgFileName;
    /// \brief stores the content of the vertex shader file
    smString vertexShaderContent;
    /// \brief stores the content of the fragment  shader file
    smString fragmentShaderContent;
    /// \brief stores the content of the geometry shader file
    smString geometryShaderContent;
    /// \brief if the vertex shader exists this will be true
    smBool vertexProgramExist;
    /// \brief if the fragment shader exists this will be true
    smBool fragmentProgramExist;
    /// \brief if the geometry shader exists this will be true
    smBool geometryProgramExist;
    /// \brief stores the parameters for vertex shader
    std::vector<smString> vertexShaderParamsString;
    /// \brief stores the parameters for fragment shader
    std::vector<smString> fragmentShaderParamsString;
    /// \brief stores the parameters for geometry shader
    std::vector<smString> geometryShaderParamsString;
    /// \brief stores the attribute parameters
    std::vector<smString> attribParamsString;
    /// \brief time for periodically checnking the shader
    smTimer time;
    std::unordered_multimap<smInt, smTextureShaderAssignment> texAssignments;
    smString modelViewMatrixName;
    smString projectionMatrixName;

    void getAttribAndParamLocations();

#ifdef SIMMEDTK_OPENGL_SHADER
    /// \brief vertex shader object
    GLhandleARB vertexShaderObject;
    /// \brief fragment  shader object
    GLhandleARB fragmentShaderObject;
    /// \brief fragment  shader object
    GLhandleARB geometryShaderObject;
    /// \brief shader program object.
    GLhandleARB shaderProgramObject;
    /// \brief stores the id of the parameters in vertex shader
    std::vector<GLint> vertexShaderParams;
    /// \brief stores the id of the parameters in fragment shader
    std::vector<GLint> fragmentShaderParams;
    /// \brief stores the id of the parameters in geometry shader
    std::vector<GLint> geometryShaderParams;
    /// \brief stores the id of the parameters in geometry shader
    std::vector<GLint> attribShaderParams;

    /// \brief creates GLSL vertex shader
    void createVertexShaderGLSL();
    /// \brief creates GLSL fragment shader
    void createFragmentShaderGLSL();
    /// \brief creates GLSL geometry shader
    void createGeometryShaderGLSL();

    /// \brief reloads the vertex shader
    void reloadVertexShaderGLSL();
    /// \brief reloads the fragment shader
    void reloadFragmentShaderGLSL();
    /// \brief reloads the geometry shader
    void reloadGeometryShaderGLSL();

    /// \brief Retrieves a shader uniform value location and stores it
    ///
    /// \param p_paramName Name of the uniform value to find
    /// \param p_shaderProgramObject Shader object to find the uniform in
    /// \param p_shaderParamsString List of shader parameter names to append to
    /// if the uniform variable is found
    /// \param p_shaderParams List of shader parameter locations to append to
    /// if the uniform variable is found
    smGLInt addShaderParamGLSL(const smString& p_paramName,
                               const GLhandleARB p_shaderProgramObject,
                               std::vector<smString>& p_shaderParamsString,
                               std::vector<GLint>& p_shaderParams);
    /// \brief add vertex, fragment and geometry shaders
    GLint addVertexShaderParamGLSL(const smString& p_paramVertex);
    GLint addFragmentShaderParamGLSL(const smString& p_paramFragment);
    GLint addGeometryShaderParamGLSL(const smString& p_paramGeometry);

public:
    /// \brief returns the program object
    GLuint getProgramObject()
    {
        return shaderProgramObject;
    }
    /// \brief returns vertex shader object
    GLuint getVertexShaderObject()
    {
        return vertexShaderObject;
    }
    /// \brief retursn fragment shader object
    GLuint getFragmentShaderObject()
    {
        return fragmentShaderObject;
    }
    /// \brief returns geomtry shader object
    GLuint getGeometryShaderObject()
    {
        return geometryShaderObject;
    }

    /// \brief Attaches The texture ID to the mesh
    void attachTexture(smUnifiedID p_meshID, smInt p_textureID);

    /// \brief assigns the texture by name if you don't know the textureID
    smBool attachTexture(smUnifiedID p_meshID, const smString& p_textureName, const smString& p_textureShaderName);

protected:
    /// \brief This stores the opengl binded texture id
    std::unordered_map<smString, smGLInt> textureGLBind;
    void autoGetTextureIds();
#endif

protected:
    /// \brief Creates a shader object from shader content,
    /// then adds the shader object to a program object for use
    ///
    /// \param p_shaderObject An empty shader handle to be assigned the value of
    /// a newly created shader
    /// \param p_shaderProgramObject A shader program to add the newly created
    /// shader to
    /// \param p_shaderContent A string containing the shader program
    /// \param p_shaderType The type of shader, can be:
    /// GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER,
    /// GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER
    void createShaderGLSL(GLhandleARB &p_shaderObject,
                          const GLhandleARB p_shaderProgramObject,
                          const smString& p_shaderContent,
                          GLenum p_shaderType);
    /// \brief Reloads and recompiles the shader object
    ///
    /// \param p_shaderObject Shader handle to be reloaded
    /// \param p_shaderContent A string containing the shader program
    void reloadShaderGLSL(const GLhandleARB p_shaderObject,
                          const smString& p_shaderContent);
/// \brief reloads all shaders
    smBool reLoadAllShaders();
/// \brief check opengl error
    smBool checkGLError();

public:
    /// \brief if the error check is enabled or not. If it is checked, opengl errors are queried and if there is, they will be stored in logger
    smBool checkErrorEnabled;
    /// \brief constructor gets the error log class
    smShader(smErrorLog *log = NULL);

    /// \brief initialized the shaders.
    /// \param vertexProgFileName   vertex program file name
    /// \param fragmentProgFileName fragment program file name
    /// \param geometryProgFileName geometry program file name
    smBool initShaders(const smString& p_vertexProgFileName,
                       const smString& p_fragmentProgFileName,
                       const smString& p_geometryProgFileName);

    /// \brief  enables the shader
    void enableShader();

    /// \brief  disables the shader
    void disableShader();

    /// \brief Initialize the shader..This is called automatically.
    virtual void initDraw(const smDrawParam &p_param);

#ifdef SIMMEDTK_OPENGL_SHADER
    /// \brief add parameter for Vertex Shader
    GLint addVertexShaderParam(const smString& p_paramVertex);
    /// \brief add parameter for Fragment Shader
    GLint addFragmentShaderParam(const smString& p_paramFragment);
    /// \brief add parameter for Geometry Shader
    GLint addGeometryShaderParam(const smString& p_paramGeometry);
    /// \brief add parameter for all Shaders
    GLint addShaderParamForAll(const smString& p_paramName);
    /// \brief attrib parameters for Shaders
    GLint addShaderParamAttrib(const smString& p_paramName);

    void createTextureParam(const smString& p_textureNameInShaderCode);

    smBool setShaderFileName(const smString& p_vertexFileName,
                             const smString& p_geometryFileName,
                             const smString& p_fragmentFileName);

    smBool setModelViewMatrixShaderName(const smString& p_modelviewMatrixName);

    smBool setProjectionMatrixShaderName(const smString& p_projectionName);

    /// \brief holds the unitform location for projection matrix. That is needed in newer versions of GLSL
    smGLInt projectionMatrix;

    /// \brief holds the unitform location for modelview matrix. That is needed in newer versions of GLSL
    smGLInt modelViewMatrix;

    void updateGLSLMatwithOPENGL();

    /// \brief returns the shader attrrib param
    smGLInt getShaderParamForAll(const smString& p_paramName);

    /// \brief returns the fragment shader uniform param
    smGLInt getFragmentShaderParam(const smString& p_paramName);

    /// \brief returns the attrib location param
    smGLInt getShaderAtrribParam(const smString& p_paramName);

    void createParam(const smString& p_param);
    smInt createAttrib(const smString& p_attrib);

    smGLInt queryUniformLocation(const smString& p_param);

#endif

    /// \brief checks whether the shader routine is updated
    smBool checkShaderUpdate(smInt milliseconds);

    /// \brief turn on/off the error checking
    void enableCheckingErrors(smBool p_checkError);

    /// \brief cleans up of the shader objects
    ~smShader()
    {
#ifdef SIMMEDTK_OPENGL_SHADER

        if (vertexProgramExist)
        {
            glDeleteObjectARB(vertexShaderObject);
        }

        if (fragmentProgramExist)
        {
            glDeleteObjectARB(fragmentShaderObject);
        }

        if (geometryProgramExist)
        {
            glDeleteObjectARB(geometryShaderObject);
        }

#endif
    }

    /// \brief void implementations for virtual functions. needs to be overwritten for any specific uniform bindings
    virtual void predraw(smMesh */*mesh*/){};
    virtual void predraw(smSurfaceMesh */*mesh*/){};
    virtual void posdraw(smMesh */*mesh*/){};
    virtual void posdraw(smSurfaceMesh */*mesh*/){};

    static smShader * getShader(smUnifiedID p_shaderID);

    smBool readShaderContent(const smString& p_file, smString& p_content);
    static void initGLShaders(smDrawParam p_param);
    void activeGLTextures(smUnifiedID p_id);
    void activeGLVertAttribs(smInt p_id, smVec3f *p_vecs, smInt p_size);
    void registerShader();
    void print() const;

    /// \brief stores the current Active shader.
    static smShader *currentShader;

    /// \brief  It is also used to save and restore the current shader is disabled for a while to use
    ///default opengl rendering. The function saveAndDisableCurrent/restoreAndEnable perform save and restore the functions.
    static smShader * savedShader;
    /// \brief if the currentShader is enabled or not
    static smBool currentShaderEnabled;
    /// \brief restores the shader from the last saved one
    static void restoreAndEnableCurrent();
    /// \brief save the last active shader and disable it.
    static void saveAndDisableCurrent();
};

#endif
