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

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smMatrix44.h"
#include "smUtilities/smTimer.h"

#include <string.h>
#include <unordered_map>
#include <list>
#include <vector>

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

    smChar vertexProgFileName[SIMMEDTK_MAX_FILENAME_LENGTH];
    smChar fragmentProgFileName[SIMMEDTK_MAX_FILENAME_LENGTH];
    smChar geometryProgFileName[SIMMEDTK_MAX_FILENAME_LENGTH];
    /// \brief Error Loging
    smErrorLog *log;
    /// \brief stores the content of the vertex shader file
    smChar *vertexShaderContent;
    /// \brief stores the content of the fragment  shader file
    smChar *fragmentShaderContent;
    /// \brief stores the content of the geometry shader file
    smChar *geometryShaderContent;
    /// \brief if the vertex shader exists this will be true
    smBool vertexProgramExist;
    /// \brief if the fragment shader exists this will be true
    smBool fragmentProgramExist;
    /// \brief if the geometry shader exists this will be true
    smBool geometryProgramExist;
    /// \brief stores the parameters for vertex shader
    std::vector<smChar*> vertexShaderParamsString;
    /// \brief stores the parameters for fragment shader
    std::vector<smChar*> fragmentShaderParamsString;
    /// \brief stores the parameters for geometry shader
    std::vector<smChar*> geometryShaderParamsString;
    /// \brief stores the attribute parameters
    std::vector<smChar*> attribParamsString;
    /// \brief error text for querying the opengl errors mostly
    smChar errorText[SIMMEDTK_MAX_ERRORLOG_TEXT];
    /// \brief time for periodically checnking the shader
    smTimer time;
    std::unordered_multimap<smInt, smTextureShaderAssignment> texAssignments;
    smChar modelViewMatrixName[SIMMEDTK_MAX_SHADERVARIABLENAME];
    smChar projectionMatrixName[SIMMEDTK_MAX_SHADERVARIABLENAME];

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

    /// \brief add vertex, fragment and geometry shaders
    GLint addVertexShaderParamGLSL(smChar* p_paramVertex);
    GLint addFragmentShaderParamGLSL(smChar* p_paramFragment);
    GLint addGeometryShaderParamGLSL(smChar* p_paramGeometry);

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
    smBool attachTexture(smUnifiedID p_meshID, smChar * const p_textureName, smChar * const p_textureShaderName);

protected:
    /// \brief This stores the opengl binded texture id
    std::unordered_map<smString, smGLInt> textureGLBind;
    void autoGetTextureIds();
#endif

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
    smBool initShaders(smChar *vertexProgFileName, smChar *fragmentProgFileName, smChar *geometryProgFileName);

    /// \brief  enables the shader
    void enableShader();

    /// \brief  disables the shader
    void disableShader();

    /// \brief Initialize the shader..This is called automatically.
    virtual void initDraw(smDrawParam p_param);

#ifdef SIMMEDTK_OPENGL_SHADER
    /// \brief add parameter for Vertex Shader
    GLint addVertexShaderParam(smChar* p_paramVertex);
    /// \brief add parameter for Fragment Shader
    GLint addFragmentShaderParam(smChar* p_paramFragment);
    /// \brief add parameter for Geometry Shader
    GLint addGeometryShaderParam(smChar* p_paramGeometry);
    /// \brief add parameter for all Shaders
    GLint addShaderParamForAll(smChar* p_paramName);
    /// \brief attrib parameters for Shaders
    GLint addShaderParamAttrib(smChar* p_paramName);

    void createTextureParam(smChar *p_textureNameInShaderCode);

    smBool setShaderFileName(smChar *p_vertexFileName, smChar *p_geometryFileName, smChar *p_fragmentFileName);
    smBool setModelViewMatrixShaderName(smChar *p_modelviewMatrixName)
    {
        if (strlen(p_modelviewMatrixName) > SIMMEDTK_MAX_SHADERVARIABLENAME - 1)
        {
            return false;
        }
        else
        {
            strcpy(this->modelViewMatrixName, p_modelviewMatrixName);
        }

        createParam(modelViewMatrixName);
        return true;
    }

    smBool setProjectionMatrixShaderName(smChar *p_projectionName)
    {
        if (strlen(p_projectionName) > SIMMEDTK_MAX_SHADERVARIABLENAME - 1)
        {
            return false;
        }
        else
        {
            strcpy(this->projectionMatrixName, p_projectionName);
        }

        createParam(projectionMatrixName);
        return true;
    }

    /// \brief holds the unitform location for projection matrix. That is needed in newer versions of GLSL
    smGLInt projectionMatrix;

    /// \brief holds the unitform location for modelview matrix. That is needed in newer versions of GLSL
    smGLInt modelViewMatrix;

    inline void updateGLSLMatwithOPENGL()
    {
        smMatrix44f proj, model;
        smGLUtils::queryModelViewMatrix<float>(model);
        smGLUtils::queryProjectionMatrix<float>(proj);

        //as the our matrix is row major, we need transpose it. Transpose parameters are true
        glUniformMatrix4fv(modelViewMatrix, 1, true, (GLfloat*)model.e);
        glUniformMatrix4fv(projectionMatrix, 1, true, (GLfloat*)proj.e);
    }

    /// \brief returns the shader attrrib param
    smGLInt getShaderParamForAll(smChar *p_paramName);

    /// \brief returns the fragment shader uniform param
    smGLInt getFragmentShaderParam(smChar *p_paramName);

    /// \brief returns the attrib location param
    smGLInt getShaderAtrribParam(smChar *p_paramName);

    void createParam(smChar * const p_param);
    smInt createAttrib(smChar * const p_attrib);

    smGLInt queryUniformLocation(smChar *const p_param)
    {
        return glGetUniformLocation(shaderProgramObject, p_param);
    }

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
    virtual void predraw(smMesh *mesh) {}
    virtual void predraw(smSurfaceMesh *mesh) {}
    virtual void posdraw(smMesh *mesh) {}
    virtual void posdraw(smSurfaceMesh *mesh) {}

    static inline smShader * getShader(smUnifiedID p_shaderID)
    {
        return shaders[p_shaderID.ID];
    }

    static void initGLShaders(smDrawParam p_param);
    void activeGLTextures(smUnifiedID p_id);
    void activeGLVertAttribs(smInt p_id, smVec3f *p_vecs, smInt p_size);
    void registerShader();
    void print();

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
