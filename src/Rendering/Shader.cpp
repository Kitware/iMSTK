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

// STL includes
#include <fstream>
#include <iostream>
#include <chrono>

// SimMedTK includes
#include "Shader.h"
#include "TextureManager.h"

std::unordered_map<int, std::shared_ptr<smShader>> smShader::shaders;
std::shared_ptr<smShader> smShader::currentShader = nullptr;
std::shared_ptr<smShader> smShader::savedShader = nullptr;

void printInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    char *infoLog;

    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

    if (infologLength > 0)
    {
        infoLog = new char[infologLength];
        glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        delete [] infoLog;
    }
}

smShader::smShader(std::shared_ptr<smErrorLog> logger)
{
    type = core::ClassType::Shader;
    log = logger;
    checkErrorEnabled = true;
    setModelViewMatrixShaderName("ModelMatrix");
    setProjectionMatrixShaderName("ProjectionMatrix");
    currentShaderEnabled = false;
    time.start();

    tangentAttrib = 0;
    projectionMatrix = 0;
    modelViewMatrix = 0;

    vertexProgFileName = "";
    fragmentProgFileName = "";
    geometryProgFileName = "";
    vertexShaderContent = "";
    fragmentShaderContent = "";
    geometryShaderContent = "";
    modelViewMatrixName = "";
    projectionMatrixName = "";

    vertexProgramExist = false;
    fragmentProgramExist = false;
    geometryProgramExist = false;
    currentShaderEnabled = false;
    checkErrorEnabled = false;
}

bool smShader::readShaderContent(const std::string& p_file, std::string& p_content)
{
    std::ifstream file;
    if ("" != p_file)
    {
        file.open(p_file);
    }
    else
    {
        return false;
    }

    if (file.fail())
    {
        if (nullptr != log)
        {
            log->addError("Shader: Shader file: " + p_file + " couldn't be opened");
        }
        return false;
    }

    file.seekg(0, std::ios::end);
    p_content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&p_content[0], p_content.size());

    file.close();
    return true;
}

///this function gets the vertex,fragment and geometry shader fileNames respectively. if you don't use
/// one of them just simply send NULL pointer as a parameter.
bool smShader::initShaders(const std::string& p_vertexProgFileName,
                             const std::string& p_fragmentProgFileName,
                             const std::string& p_geometryProgFileName)
{
    if (glewIsSupported("GL_VERSION_2_0") == GL_FALSE)
    {
        if (log != NULL)
        {
            log->addError("Shader:OpenGL 2.0 not supported");
        }

        return false;
    }

    shaderProgramObject = glCreateProgram();

    if ("" != p_vertexProgFileName)
    {
        if (readShaderContent(p_vertexProgFileName, this->vertexShaderContent))
        {
            this->vertexProgFileName = p_vertexProgFileName;
        }
        else
        {
            return false;
        }

#ifdef SIMMEDTK_OPENGL_SHADER
        createVertexShaderGLSL();
        vertexShaderContent.clear(); //No need for the contents anymore
        checkGLError();
#endif
        vertexProgramExist = true;
    }
    else
    {
        vertexShaderObject = 0;
        vertexProgramExist = false;
    }

    if ("" != p_fragmentProgFileName)
    {
        if (readShaderContent(p_fragmentProgFileName, this->fragmentShaderContent))
        {
            this->fragmentProgFileName = p_fragmentProgFileName;
        }
        else
        {
            return false;
        }
#ifdef SIMMEDTK_OPENGL_SHADER
        createFragmentShaderGLSL();
        fragmentShaderContent.clear();
        checkGLError();
        fragmentProgramExist = true;
#endif
    }
    else
    {
        fragmentShaderObject = 0;
        fragmentProgramExist = false;
    }

    if ("" != p_geometryProgFileName)
    {
        if (readShaderContent(p_geometryProgFileName, this->geometryShaderContent))
        {
            this->geometryProgFileName = p_geometryProgFileName;
        }
        else
        {
            return false;
        }
#ifdef SIMMEDTK_OPENGL_SHADER
        createGeometryShaderGLSL();
        geometryShaderContent.clear();
        checkGLError();

        geometryProgramExist = true;
        //note that:based on geometry shader needw, the input and output parameters for geometry shader below may change
        glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
        glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
        glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_VERTICES_OUT_EXT, 1024);
#endif
    }
    else
    {
        geometryShaderObject = 0;
        geometryProgramExist = false;
    }

    glLinkProgram(shaderProgramObject);
    printInfoLog(shaderProgramObject);
    modelViewMatrix = glGetUniformLocation(shaderProgramObject, modelViewMatrixName.data());
    projectionMatrix = glGetUniformLocation(shaderProgramObject, projectionMatrixName.data());

    return true;
}

void smShader::createShaderGLSL(GLhandleARB &p_shaderObject,
                                const GLhandleARB p_shaderProgramObject,
                                const std::string& p_shaderContent,
                                GLenum p_shaderType)
{
    const char *shaderSrc = p_shaderContent.data();
    p_shaderObject = glCreateShader(p_shaderType);
    glShaderSource(p_shaderObject, 1, &shaderSrc, NULL);
    glCompileShader(p_shaderObject);
    printInfoLog(p_shaderObject);
    checkGLError();

    glAttachShader(p_shaderProgramObject, p_shaderObject);
    checkGLError();
}

void smShader::createVertexShaderGLSL()
{
    createShaderGLSL(vertexShaderObject, shaderProgramObject,
                     vertexShaderContent, GL_VERTEX_SHADER);
}

void smShader::createFragmentShaderGLSL()
{
    createShaderGLSL(fragmentShaderObject, shaderProgramObject,
                     fragmentShaderContent, GL_FRAGMENT_SHADER);
}
void smShader::createGeometryShaderGLSL()
{
    createShaderGLSL(geometryShaderObject, shaderProgramObject,
                     geometryShaderContent, GL_GEOMETRY_SHADER_EXT);
}

void smShader::reloadShaderGLSL(const GLhandleARB p_shaderObject,
                                const std::string& p_shaderContent)
{
    const char *shaderSrc = p_shaderContent.data();
    glShaderSource(p_shaderObject, 1, &shaderSrc, NULL);
    glCompileShader(p_shaderObject);
    checkGLError();
}

void smShader::reloadVertexShaderGLSL()
{
    reloadShaderGLSL(vertexShaderObject, vertexShaderContent);
}

void smShader::reloadFragmentShaderGLSL()
{
    reloadShaderGLSL(fragmentShaderObject, fragmentShaderContent);
}

void smShader::reloadGeometryShaderGLSL()
{
    reloadShaderGLSL(geometryShaderObject, geometryShaderContent);
}

///checks the opengl error
bool smShader::checkGLError()
{
    std::string errorText;
    if (checkErrorEnabled)
    {
        if (smGLUtils::queryGLError(errorText))
        {
            if (log != NULL)
            {
                log->addError(errorText);
            }

            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}


///enable the shader
void smShader::enableShader()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (vertexProgramExist)
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
    }

    if (fragmentProgramExist)
    {
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }

    if (geometryProgramExist)
    {
        glEnable(GL_GEOMETRY_SHADER_ARB);
    }

    glUseProgramObjectARB(shaderProgramObject);
    smShader::currentShader = safeDownCast<smShader>();
    currentShaderEnabled = true;

#endif
}

///disable the shader
void smShader::disableShader()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (vertexProgramExist)
    {
        glDisable(GL_VERTEX_PROGRAM_ARB);
    }

    if (fragmentProgramExist)
    {
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }

    if (geometryProgramExist)
    {
        glDisable(GL_GEOMETRY_SHADER_ARB);
    }

    glUseProgramObjectARB(0);
    smShader::currentShader = safeDownCast<smShader>();
    currentShaderEnabled = false;
#endif
}

///enable the shader
void smShader::restoreAndEnableCurrent()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (smShader::savedShader != NULL)
    {
        smShader::currentShader = smShader::savedShader;

        if (currentShader->vertexProgramExist)
        {
            glEnable(GL_VERTEX_PROGRAM_ARB);
        }

        if (currentShader->fragmentProgramExist)
        {
            glEnable(GL_FRAGMENT_PROGRAM_ARB);
        }

        if (currentShader->geometryProgramExist)
        {
            glEnable(GL_GEOMETRY_SHADER_ARB);
        }

        glUseProgramObjectARB(currentShader->shaderProgramObject);
        currentShaderEnabled = true;
    }

#endif
}

///disable the shader
void smShader::saveAndDisableCurrent()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (currentShader != NULL)
    {
        if (smShader::currentShader->vertexProgramExist)
        {
            glDisable(GL_VERTEX_PROGRAM_ARB);
        }

        if (smShader::currentShader->fragmentProgramExist)
        {
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
        }

        if (smShader::currentShader->geometryProgramExist)
        {
            glDisable(GL_GEOMETRY_SHADER_ARB);
        }

        currentShaderEnabled = false;
        smShader::savedShader = smShader::currentShader;
        glUseProgramObjectARB(0);
    }

#endif
}

GLint smShader::addShaderParamGLSL(const std::string& p_paramName,
                                     const GLhandleARB p_shaderProgramObject,
                                     std::vector<std::string>& p_shaderParamsString,
                                     std::vector<GLint>& p_shaderParams)
{
    GLint param;
    param = glGetUniformLocation(p_shaderProgramObject, p_paramName.data());
    checkGLError();
    p_shaderParamsString.push_back(p_paramName);
    p_shaderParams.push_back(param);
    return param;
}

GLint smShader::addVertexShaderParamGLSL(const std::string& p_paramNameVertex)
{
    return addShaderParamGLSL(p_paramNameVertex, shaderProgramObject,
                              vertexShaderParamsString, vertexShaderParams);
}

GLint smShader::addFragmentShaderParamGLSL(const std::string& p_paramNameFragment)
{
    return addShaderParamGLSL(p_paramNameFragment, shaderProgramObject,
                              fragmentShaderParamsString, fragmentShaderParams);
}

GLint smShader::addGeometryShaderParamGLSL(const std::string& p_paramNameGeometry)
{
    return addShaderParamGLSL(p_paramNameGeometry, shaderProgramObject,
                              geometryShaderParamsString, geometryShaderParams);
}

GLint smShader::addVertexShaderParam(const std::string& p_paramNameVertex)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addVertexShaderParamGLSL(p_paramNameVertex);
#endif
}
GLint smShader::addFragmentShaderParam(const std::string& p_paramNameFragment)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addFragmentShaderParamGLSL(p_paramNameFragment);
#endif
}

GLint smShader::addGeometryShaderParam(const std::string& p_paramNameGeometry)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addGeometryShaderParamGLSL(p_paramNameGeometry);
#endif
}

GLint smShader::addShaderParamForAll(const std::string& p_paramName)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    GLint param;
    param = glGetUniformLocation(shaderProgramObject, p_paramName.data());
    vertexShaderParamsString.push_back(p_paramName);
    vertexShaderParams.push_back(param);

    fragmentShaderParamsString.push_back(p_paramName);
    fragmentShaderParams.push_back(param);

    geometryShaderParamsString.push_back(p_paramName);
    geometryShaderParams.push_back(param);

    textureGLBind[p_paramName] = param;
    return param;
#endif
}

GLint smShader::getShaderParamForAll(const std::string& p_paramName) const
{
#ifdef SIMMEDTK_OPENGL_SHADER
    for (size_t i = 0; i < vertexShaderParamsString.size(); i++)
    {
        if (vertexShaderParamsString[i] == p_paramName)
        {
            return vertexShaderParams[i];
        }
    }

    return -1;
#endif
}

GLint smShader::getFragmentShaderParam(const std::string& p_paramName) const
{
#ifdef SIMMEDTK_OPENGL_SHADER
    for (size_t i = 0; i < fragmentShaderParamsString.size(); i++)
    {
        if (fragmentShaderParamsString[i] == p_paramName)
        {
            return fragmentShaderParams[i];
        }
    }

    return -1;
#endif
}

GLint smShader::getShaderAtrribParam(const std::string& p_paramName) const
{
#ifdef SIMMEDTK_OPENGL_SHADER
    for (size_t i = 0; i < attribParamsString.size(); i++)
    {
        if (attribParamsString[i] == p_paramName)
        {
            return attribShaderParams[i];
        }
    }

    return -1;
#endif
}
GLint smShader::addShaderParamAttrib(const std::string& p_paramName)
{

    GLint param;
    param = glGetAttribLocationARB(shaderProgramObject, p_paramName.data());

    checkGLError();

    return param;
}

bool smShader::reLoadAllShaders()
{

    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;
    std::ifstream geometryShaderFile;

    if (vertexProgramExist == true)
    {
        if (false == readShaderContent(this->vertexProgFileName, this->vertexShaderContent))
        {
            return false;
        }
#ifdef SIMMEDTK_OPENGL_SHADER
        reloadVertexShaderGLSL();
        vertexShaderContent.clear();

        checkGLError();

#endif
    }
    else
    {
        vertexShaderObject = 0;
    }

    if (fragmentProgramExist == true)
    {
        if (false == readShaderContent(this->fragmentProgFileName, this->fragmentShaderContent))
        {
            return false;
        }
#ifdef SIMMEDTK_OPENGL_SHADER
        reloadFragmentShaderGLSL();
        fragmentShaderContent.clear();

        checkGLError();

#endif
    }
    else
    {
        fragmentShaderObject = 0;
    }


    if (geometryProgramExist == true)
    {
        if (false == readShaderContent(this->geometryProgFileName, this->geometryShaderContent))
        {
            return false;
        }
#ifdef SIMMEDTK_OPENGL_SHADER
        reloadGeometryShaderGLSL();
        geometryShaderContent.clear();

        checkGLError();

#endif
    }
    else
    {
        geometryShaderObject = 0;
    }

    glLinkProgram(shaderProgramObject);

    checkGLError();

    return true;
}
///checks the shader source code within the given interval in milliseconds
bool smShader::checkShaderUpdate(int interval)
{
    if ((time.elapsed() * 1000) > interval)
    {
        time.start();
        return reLoadAllShaders();
    }
    return true;
}

void smShader::enableCheckingErrors(bool p_checkError)
{

    this->checkErrorEnabled = p_checkError;
}

void smShader::attachTexture(std::shared_ptr<smUnifiedId> p_meshID, int p_textureID)
{

    smTextureShaderAssignment assign;
    assign.textureId = p_textureID;
    texAssignments.insert( {p_meshID->getId(), assign} );
}

bool smShader::attachTexture(std::shared_ptr<smUnifiedId> p_meshID,
                               const std::string& p_textureName,
                               const std::string& p_textureShaderName)
{

    smTextureShaderAssignment assign;

    if (smTextureManager::findTextureId(p_textureName, assign.textureId) == SIMMEDTK_TEXTURE_NOTFOUND)
    {
        std::cout << "texture " << p_textureName << " is not found in shader:" << p_textureShaderName << "  for mesh id:" << p_meshID->getId() <<  "\n";
        return false;
    }

    assign.shaderParamName = p_textureShaderName;
    texAssignments.insert( {p_meshID->getId(), assign} );

    return true;
}

void smShader::autoGetTextureIds()
{

    std::unordered_multimap<int, smTextureShaderAssignment>::iterator i = texAssignments.begin() ;

    for (; i != texAssignments.end(); i++)
    {
        i->second.textureShaderGLassignment = textureGLBind[i->second.shaderParamName];
    }
}

void smShader::createTextureParam(const std::string& p_textureNameInShaderCode)
{

    this->textureGLBind[p_textureNameInShaderCode] = -1;
}

bool smShader::setShaderFileName(const std::string& p_vertexFileName,
                                   const std::string& p_geometryFileName,
                                   const std::string& p_fragmentFileName)
{

    if ("" != p_vertexFileName)
    {
        if (core::MaxFilenameLength < p_vertexFileName.length())
        {
            if (nullptr != log)
            {
                log->addError("Vertex Shader filename is longer than max file length");
                return false;
            }
        }

        vertexProgFileName = p_vertexFileName;
    }

    if ("" != p_geometryFileName)
    {
        if (core::MaxFilenameLength < geometryProgFileName.length())
        {
            if (nullptr != log)
            {
                log->addError("Geometry Shader filename is longer than max file length");
                return false;
            }
        }

        geometryProgFileName = p_geometryFileName;
    }

    if ("" != p_fragmentFileName)
    {
        if (core::MaxFilenameLength < fragmentProgFileName.length())
        {
            if (nullptr != log)
            {
                log->addError("Fragment Shader filename is longer than max file length");
                return false;
            }
        }

        fragmentProgFileName = p_fragmentFileName;
    }

    return true;
}

void smShader::initDraw()
{

    initShaders(vertexProgFileName, fragmentProgFileName, geometryProgFileName);
    getAttribAndParamLocations();
    autoGetTextureIds();
}

int smShader::createAttrib(const std::string& p_attrib)
{
    attribParamsString.push_back(p_attrib);
    return attribParamsString.size();
}

void smShader::createParam(const std::string& p_param)
{
    vertexShaderParamsString.push_back(p_param);
    fragmentShaderParamsString.push_back(p_param);
    geometryShaderParamsString.push_back(p_param);
}

void smShader::getAttribAndParamLocations()
{

    GLint param;

    for (size_t i = 0; i < vertexShaderParamsString.size(); i++)
    {
        param = glGetUniformLocation(shaderProgramObject, vertexShaderParamsString[i].data());
        vertexShaderParams.push_back(param);

        if (textureGLBind[vertexShaderParamsString[i]] != -1)
        {
            textureGLBind[vertexShaderParamsString[i]] = param;
        }
    }

    for (size_t i = 0; i < fragmentShaderParamsString.size(); i++)
    {
        param = glGetUniformLocation(shaderProgramObject, fragmentShaderParamsString[i].data());
        fragmentShaderParams.push_back(param);
        std::cout << "[smShader::getAttribAndParamLocations] " << fragmentShaderParamsString[i] << " " << param << "\n";

        if (textureGLBind[fragmentShaderParamsString[i]] != -1)
        {
            textureGLBind[fragmentShaderParamsString[i]] = param;
        }
    }

    for (size_t i = 0; i < geometryShaderParamsString.size(); i++)
    {
        param = glGetUniformLocation(shaderProgramObject, geometryShaderParamsString[i].data());
        geometryShaderParams.push_back(param);

        if (textureGLBind[geometryShaderParamsString[i]] != -1)
        {
            textureGLBind[geometryShaderParamsString[i]] = param;
        }
    }

    for (size_t i = 0; i < attribParamsString.size(); i++)
    {
        param = glGetAttribLocation(shaderProgramObject, attribParamsString[i].data());
        attribShaderParams.push_back(param);
    }
}


void smShader::initGLShaders()
{
    for(auto& x : shaders)
        x.second->initDraw();
}

void smShader::activeGLTextures(std::shared_ptr<smUnifiedId> p_id)
{
    int counter = 0;
    auto range = texAssignments.equal_range(p_id->getId());

    for (auto i = range.first; i != range.second; i++)
    {
        smTextureManager::activateTexture(i->second.textureId, counter);
        glUniform1iARB(i->second.textureShaderGLassignment, counter);
        counter++;
    }
}

void smShader::activeGLVertAttribs(int p_id, smVec3d *p_vecs, int /*p_size*/)
{
    glVertexAttribPointer(attribShaderParams[p_id], 3, GL_FLOAT, GL_FALSE, 0, p_vecs);
}
void smShader::registerShader()
{
    shaders[this->getUniqueId()->getId()] = safeDownCast<smShader>();
}

void smShader::print() const
{
    for (size_t i = 0; i < vertexShaderParamsString.size(); i++)
    {
        std::cout << "Param:" << vertexShaderParamsString[i] << "\n";
    }
}
bool smShader::setModelViewMatrixShaderName(const std::string& p_modelviewMatrixName )
{
    if ((core::MaxShaderVariableName - 1) < p_modelviewMatrixName.length())
    {
        return false;
    }
    else
    {
        this->modelViewMatrixName = p_modelviewMatrixName;
    }

    createParam(modelViewMatrixName);
    return true;
}
bool smShader::setProjectionMatrixShaderName(const std::string& p_projectionName)
{
    if ((core::MaxShaderVariableName - 1) < p_projectionName.length())
    {
        return false;
    }
    else
    {
        this->projectionMatrixName = p_projectionName;
    }

    createParam(projectionMatrixName);
    return true;
}
void smShader::updateGLSLMatwithOPENGL()
{
    smMatrix44f proj, model;
    smGLUtils::queryModelViewMatrix( model );
    smGLUtils::queryProjectionMatrix( proj );

    //as the our matrix is row major, we need transpose it. Transpose parameters are true
    glUniformMatrix4fv( modelViewMatrix, 1, true, model.data() );
    glUniformMatrix4fv( projectionMatrix, 1, true, proj.data() );
}
GLint smShader::queryUniformLocation(const std::string& p_param)
{
    return glGetUniformLocation(shaderProgramObject, p_param.data());
}
std::shared_ptr<smShader> smShader::getShader( std::shared_ptr<smUnifiedId> p_shaderID )
{
    return shaders[p_shaderID->getId()];
}
smShader::~smShader()
{
#ifdef SIMMEDTK_OPENGL_SHADER

    if(vertexProgramExist)
    {
        glDeleteObjectARB(vertexShaderObject);
    }

    if(fragmentProgramExist)
    {
        glDeleteObjectARB(fragmentShaderObject);
    }

    if(geometryProgramExist)
    {
        glDeleteObjectARB(geometryShaderObject);
    }

#endif
}
