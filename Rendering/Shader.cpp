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

#include "Rendering/Shader.h"

// STL includes
#include <fstream>
#include <iostream>
#include <chrono>

// SimMedTK includes
#include "TextureManager.h"

std::unordered_map<int, std::shared_ptr<Shader>> Shader::shaders;
std::shared_ptr<Shader> Shader::currentShader = nullptr;
std::shared_ptr<Shader> Shader::savedShader = nullptr;

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

Shader::Shader(std::shared_ptr<ErrorLog> logger)
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

bool Shader::readShaderContent(const std::string& p_file, std::string& p_content)
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
/// one of them just simply send nullptr pointer as a parameter.
bool Shader::initShaders(const std::string& p_vertexProgFileName,
                             const std::string& p_fragmentProgFileName,
                             const std::string& p_geometryProgFileName)
{
    if (glewIsSupported("GL_VERSION_2_0") == GL_FALSE)
    {
        if (log != nullptr)
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

void Shader::createShaderGLSL(GLhandleARB &p_shaderObject,
                                const GLhandleARB p_shaderProgramObject,
                                const std::string& p_shaderContent,
                                GLenum p_shaderType)
{
    const char *shaderSrc = p_shaderContent.data();
    p_shaderObject = glCreateShader(p_shaderType);
    glShaderSource(p_shaderObject, 1, &shaderSrc, nullptr);
    glCompileShader(p_shaderObject);
    printInfoLog(p_shaderObject);
    checkGLError();

    glAttachShader(p_shaderProgramObject, p_shaderObject);
    checkGLError();
}

void Shader::createVertexShaderGLSL()
{
    createShaderGLSL(vertexShaderObject, shaderProgramObject,
                     vertexShaderContent, GL_VERTEX_SHADER);
}

void Shader::createFragmentShaderGLSL()
{
    createShaderGLSL(fragmentShaderObject, shaderProgramObject,
                     fragmentShaderContent, GL_FRAGMENT_SHADER);
}
void Shader::createGeometryShaderGLSL()
{
    createShaderGLSL(geometryShaderObject, shaderProgramObject,
                     geometryShaderContent, GL_GEOMETRY_SHADER_EXT);
}

void Shader::reloadShaderGLSL(const GLhandleARB p_shaderObject,
                                const std::string& p_shaderContent)
{
    const char *shaderSrc = p_shaderContent.data();
    glShaderSource(p_shaderObject, 1, &shaderSrc, nullptr);
    glCompileShader(p_shaderObject);
    checkGLError();
}

void Shader::reloadVertexShaderGLSL()
{
    reloadShaderGLSL(vertexShaderObject, vertexShaderContent);
}

void Shader::reloadFragmentShaderGLSL()
{
    reloadShaderGLSL(fragmentShaderObject, fragmentShaderContent);
}

void Shader::reloadGeometryShaderGLSL()
{
    reloadShaderGLSL(geometryShaderObject, geometryShaderContent);
}

///checks the opengl error
bool Shader::checkGLError()
{
    std::string errorText;
    if (checkErrorEnabled)
    {
        if (GLUtils::queryGLError(errorText))
        {
            if (log != nullptr)
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
void Shader::enableShader()
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
    Shader::currentShader = safeDownCast<Shader>();
    currentShaderEnabled = true;

#endif
}

///disable the shader
void Shader::disableShader()
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
    Shader::currentShader = safeDownCast<Shader>();
    currentShaderEnabled = false;
#endif
}

///enable the shader
void Shader::restoreAndEnableCurrent()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (Shader::savedShader != nullptr)
    {
        Shader::currentShader = Shader::savedShader;

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
void Shader::saveAndDisableCurrent()
{

#ifdef SIMMEDTK_OPENGL_SHADER

    if (currentShader != nullptr)
    {
        if (Shader::currentShader->vertexProgramExist)
        {
            glDisable(GL_VERTEX_PROGRAM_ARB);
        }

        if (Shader::currentShader->fragmentProgramExist)
        {
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
        }

        if (Shader::currentShader->geometryProgramExist)
        {
            glDisable(GL_GEOMETRY_SHADER_ARB);
        }

        currentShaderEnabled = false;
        Shader::savedShader = Shader::currentShader;
        glUseProgramObjectARB(0);
    }

#endif
}

GLint Shader::addShaderParamGLSL(const std::string& p_paramName,
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

GLint Shader::addVertexShaderParamGLSL(const std::string& p_paramNameVertex)
{
    return addShaderParamGLSL(p_paramNameVertex, shaderProgramObject,
                              vertexShaderParamsString, vertexShaderParams);
}

GLint Shader::addFragmentShaderParamGLSL(const std::string& p_paramNameFragment)
{
    return addShaderParamGLSL(p_paramNameFragment, shaderProgramObject,
                              fragmentShaderParamsString, fragmentShaderParams);
}

GLint Shader::addGeometryShaderParamGLSL(const std::string& p_paramNameGeometry)
{
    return addShaderParamGLSL(p_paramNameGeometry, shaderProgramObject,
                              geometryShaderParamsString, geometryShaderParams);
}

GLint Shader::addVertexShaderParam(const std::string& p_paramNameVertex)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addVertexShaderParamGLSL(p_paramNameVertex);
#endif
}
GLint Shader::addFragmentShaderParam(const std::string& p_paramNameFragment)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addFragmentShaderParamGLSL(p_paramNameFragment);
#endif
}

GLint Shader::addGeometryShaderParam(const std::string& p_paramNameGeometry)
{

#ifdef SIMMEDTK_OPENGL_SHADER
    return addGeometryShaderParamGLSL(p_paramNameGeometry);
#endif
}

GLint Shader::addShaderParamForAll(const std::string& p_paramName)
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

GLint Shader::getShaderParamForAll(const std::string& p_paramName) const
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

GLint Shader::getFragmentShaderParam(const std::string& p_paramName) const
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

GLint Shader::getShaderAtrribParam(const std::string& p_paramName) const
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
GLint Shader::addShaderParamAttrib(const std::string& p_paramName)
{

    GLint param;
    param = glGetAttribLocationARB(shaderProgramObject, p_paramName.data());

    checkGLError();

    return param;
}

bool Shader::reLoadAllShaders()
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
bool Shader::checkShaderUpdate(int interval)
{
    if ((time.elapsed() * 1000) > interval)
    {
        time.start();
        return reLoadAllShaders();
    }
    return true;
}

void Shader::enableCheckingErrors(bool p_checkError)
{

    this->checkErrorEnabled = p_checkError;
}

void Shader::attachTexture(std::shared_ptr<UnifiedId> p_meshID, int p_textureID)
{

    TextureShaderAssignment assign;
    assign.textureId = p_textureID;
    texAssignments.insert( {p_meshID->getId(), assign} );
}

bool Shader::attachTexture(std::shared_ptr<UnifiedId> p_meshID,
                               const std::string& p_textureName,
                               const std::string& p_textureShaderName)
{

    TextureShaderAssignment assign;

    if (TextureManager::findTextureId(p_textureName, assign.textureId) == SIMMEDTK_TEXTURE_NOTFOUND)
    {
        std::cout << "texture " << p_textureName << " is not found in shader:" << p_textureShaderName << "  for mesh id:" << p_meshID->getId() <<  "\n";
        return false;
    }

    assign.shaderParamName = p_textureShaderName;
    texAssignments.insert( {p_meshID->getId(), assign} );

    return true;
}

void Shader::autoGetTextureIds()
{

    std::unordered_multimap<int, TextureShaderAssignment>::iterator i = texAssignments.begin() ;

    for (; i != texAssignments.end(); i++)
    {
        i->second.textureShaderGLassignment = textureGLBind[i->second.shaderParamName];
    }
}

void Shader::createTextureParam(const std::string& p_textureNameInShaderCode)
{

    this->textureGLBind[p_textureNameInShaderCode] = -1;
}

bool Shader::setShaderFileName(const std::string& p_vertexFileName,
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

void Shader::initDraw()
{

    initShaders(vertexProgFileName, fragmentProgFileName, geometryProgFileName);
    getAttribAndParamLocations();
    autoGetTextureIds();
}

int Shader::createAttrib(const std::string& p_attrib)
{
    attribParamsString.push_back(p_attrib);
    return attribParamsString.size();
}

void Shader::createParam(const std::string& p_param)
{
    vertexShaderParamsString.push_back(p_param);
    fragmentShaderParamsString.push_back(p_param);
    geometryShaderParamsString.push_back(p_param);
}

void Shader::getAttribAndParamLocations()
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
        std::cout << "[Shader::getAttribAndParamLocations] " << fragmentShaderParamsString[i] << " " << param << "\n";

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


void Shader::initGLShaders()
{
    for(auto& x : shaders)
        x.second->initDraw();
}

void Shader::activeGLTextures(std::shared_ptr<UnifiedId> p_id)
{
    int counter = 0;
    auto range = texAssignments.equal_range(p_id->getId());

    for (auto i = range.first; i != range.second; i++)
    {
        TextureManager::activateTexture(i->second.textureId, counter);
        glUniform1iARB(i->second.textureShaderGLassignment, counter);
        counter++;
    }
}

void Shader::activeGLVertAttribs(int p_id, core::Vec3d *p_vecs, int /*p_size*/)
{
    glVertexAttribPointer(attribShaderParams[p_id], 3, GL_FLOAT, GL_FALSE, 0, p_vecs);
}
void Shader::registerShader()
{
    shaders[this->getUniqueId()->getId()] = safeDownCast<Shader>();
}

void Shader::print() const
{
    for (size_t i = 0; i < vertexShaderParamsString.size(); i++)
    {
        std::cout << "Param:" << vertexShaderParamsString[i] << "\n";
    }
}
bool Shader::setModelViewMatrixShaderName(const std::string& p_modelviewMatrixName )
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
bool Shader::setProjectionMatrixShaderName(const std::string& p_projectionName)
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
void Shader::updateGLSLMatwithOPENGL()
{
    Matrix44f proj, model;
    GLUtils::queryModelViewMatrix( model );
    GLUtils::queryProjectionMatrix( proj );

    //as the our matrix is row major, we need transpose it. Transpose parameters are true
    glUniformMatrix4fv( modelViewMatrix, 1, true, model.data() );
    glUniformMatrix4fv( projectionMatrix, 1, true, proj.data() );
}
GLint Shader::queryUniformLocation(const std::string& p_param)
{
    return glGetUniformLocation(shaderProgramObject, p_param.data());
}
std::shared_ptr<Shader> Shader::getShader( std::shared_ptr<UnifiedId> p_shaderID )
{
    return shaders[p_shaderID->getId()];
}
Shader::~Shader()
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
