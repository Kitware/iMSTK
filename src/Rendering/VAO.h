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

#ifndef SMVAO_H
#define SMVAO_H

// STL includes
#include <cassert>
#include <unordered_map>

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "ConfigRendering.h"
#include "Mesh/Mesh.h"
#include "GLUtils.h"
#include "Core/Utils.h"
#include "VBO.h"
#include "VAO.h"
#include "Shader.h"

enum smVBOBufferType
{
    SMVBO_POS,
    SMVBO_NORMALS,
    SMVBO_TEXTURECOORDS,
    SMVBO_TANGENTS,
    SMVBO_INDEX,
    SMVBO_VEC4F,
    SMVBO_VEC3F,
    SMVBO_VEC2F
};

struct smVBOBufferEntryInfo
{
    ///attrib index; 0, 1, 2. It starts from 0
    int attributeIndex;
    ///based on type the data buffer  will change. It may be Position, normals, texture coords, tangents
    smVBOBufferType arrayBufferType;
    ///pointer to the actual that. It is mesh data.
    void *attribPointer;
    ///total number of elements
    int nbrElements;
    ///total size of elements in bytes.
    int size;
    ///attribName in the shader
    std::string   shaderAttribName;
    GLint    shaderAttribLocation;
public:
    smVBOBufferEntryInfo();
};

/// \brief Vertex Array Object for fast rendering
class smVAO: public smCoreClass
{
public:
    /// \brief need error log and totalBuffer Size
    smVAO(std::shared_ptr<smErrorLog> p_log, smVBOType p_vboType = SIMMEDTK_VBO_DYNAMIC, bool p_bindShaderObjects = true);

    /// \brief set internal buffer manually. type, attrib name, number of elements and pointer to the data
    void setBufferData(smVBOBufferType p_type, std::string p_ShaderAttribName, int p_nbrElements, void *p_ptr);

    /// \brief set the triangle information
    void setTriangleInfo(std::string p_ShaderAttribName, int p_nbrTriangles, void *p_ptr);

    /// \brief fills the buffer by directly using mesh. It uses default attrib location for shader
    bool setBufferDataFromMesh(smMesh *p_mesh,
                                 std::shared_ptr<smShader> p_shader,
                                 std::string p_POSITIONShaderName = "Position",
                                 std::string p_NORMALShaderName = "Normal",
                                 std::string p_TEXTURECOORDShaderName = "texCoords",
                                 std::string p_TANGENTSName = "Tangents");
    /// \brief updates the buffer with data. It is important for meshes undergoes topology changes
    bool updateStreamData() const;

    static void initVAOs();

    /// \brief  init VAO buffers
    void initBuffers();

    /// \brief get VAO given the shader ID
    static std::shared_ptr<smVAO> getVAO(std::shared_ptr<smUnifiedId> p_shaderID);

    /// \brief  enable the vertex array object
    void enable() const;

    /// \brief disable VAO
    void disable() const;

    /// \brief draw VAO
    void draw() const override;

    /// \brief constructor
    ~smVAO();

public:
    GLuint VAO;
    int totalNbrBuffers;
    GLuint bufferIndices[SIMMEDTK_MAX_VBOBUFFERS];
    int indexBufferLocation;///stores the index buffer location in the bufferIndices array to easy access
    smVBOBufferEntryInfo bufferInfo[SIMMEDTK_MAX_VBOBUFFERS];
    smVBOType vboType;
    ///All VBOs are stored here
    static std::unordered_map<int, std::shared_ptr<smVAO>> VAOs;
    smMesh *mesh;

private:
    std::shared_ptr<smErrorLog>  log;
    std::shared_ptr<smShader> shader;
    bool renderingError;
    ///Used for attaching attribs to the vertex objects
    bool bindShaderObjects;
};

#endif
