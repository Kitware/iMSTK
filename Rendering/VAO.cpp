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

#include "Rendering/VAO.h"
#include "Rendering/Shader.h"
#include "Rendering/Viewer.h"

std::unordered_map<int, std::shared_ptr<VAO>> VAO::VAOs;

void VAO::initBuffers()
{
    std::string error;
    ///Create the Vertex Array Objects
    glGenVertexArrays(1, &vaObject);
    glBindVertexArray(vaObject);

    ///Create Vertex Buffer Objects(VBOs)
    glGenBuffers(totalNbrBuffers, bufferIndices);

    SM_CHECKERROR(log, error);

    ///Initialize and file the VBOs
    for (int i = 0; i < totalNbrBuffers; i++)
    {
        if (bufferInfo[i].arrayBufferType != SMVBO_INDEX)
        {
            glBindBuffer(GL_ARRAY_BUFFER, bufferIndices[i]);

            if (vboType == SIMMEDTK_VBO_STATIC)
            {
                glBufferData(GL_ARRAY_BUFFER, bufferInfo[i].size, bufferInfo[i].attribPointer, GL_STATIC_DRAW);
            }
            else if (vboType == SIMMEDTK_VBO_DYNAMIC)
            {
                glBufferData(GL_ARRAY_BUFFER, bufferInfo[i].size, bufferInfo[i].attribPointer, GL_DYNAMIC_DRAW);
            }
        }
        else
        {
            ///if it is index array, the buffer should be GL_ELEMENT_ARRAY_BUFFER
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIndices[i]);

            if (vboType == SIMMEDTK_VBO_STATIC)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferInfo[i].size, bufferInfo[i].attribPointer, GL_STATIC_DRAW);
            }
            else if (vboType == SIMMEDTK_VBO_DYNAMIC)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferInfo[i].size, bufferInfo[i].attribPointer, GL_DYNAMIC_DRAW);
            }

            indexBufferLocation = i;
            SM_CHECKERROR(log, error);
            continue;
        }

        if (bufferInfo[i].arrayBufferType == SMVBO_POS)
        {
            glEnableVertexAttribArray(bufferInfo[i].attributeIndex);
            glVertexAttribPointer(bufferInfo[i].attributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

            if (bindShaderObjects)
            {
                glBindAttribLocation(shader->getProgramObject(), bufferInfo[i].attributeIndex, bufferInfo[i].shaderAttribName.c_str());
            }
        }
        else if (bufferInfo[i].arrayBufferType == SMVBO_NORMALS)
        {
            glEnableVertexAttribArray(bufferInfo[i].attributeIndex);
            glVertexAttribPointer(bufferInfo[i].attributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

            if (bindShaderObjects)
            {
                glBindAttribLocation(shader->getProgramObject(), bufferInfo[i].attributeIndex, bufferInfo[i].shaderAttribName.c_str());
            }
        }
        else if (bufferInfo[i].arrayBufferType == SMVBO_TEXTURECOORDS || bufferInfo[i].arrayBufferType == SMVBO_VEC2F)
        {
            glEnableVertexAttribArray(bufferInfo[i].attributeIndex);
            glVertexAttribPointer(bufferInfo[i].attributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

            if (bindShaderObjects)
            {
                glBindAttribLocation(shader->getProgramObject(), bufferInfo[i].attributeIndex, bufferInfo[i].shaderAttribName.c_str());
            }
        }
        else if (bufferInfo[i].arrayBufferType == SMVBO_VEC4F)
        {
            glEnableVertexAttribArray(bufferInfo[i].attributeIndex);
            glVertexAttribPointer(bufferInfo[i].attributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

            if (bindShaderObjects)
            {
                glBindAttribLocation(shader->getProgramObject(), bufferInfo[i].attributeIndex, bufferInfo[i].shaderAttribName.c_str());
            }
        }

        shader->enableShader();
        shader->disableShader();
        //Check the error
        SM_CHECKERROR(log, error);
    }

    ///Go back to the default buffer state
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

///Updates the buffers. Call this function if there is change in the mesh. If it is a simulation mesh, it needs to be called in and very frame
bool VAO::updateStreamData() const
{

    if (this->vboType == SIMMEDTK_VBO_STATIC)
    {
        return false;
    }

    else if (vboType == SIMMEDTK_VBO_NOINDICESCHANGE || vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        for (int i = 0; i < totalNbrBuffers; i++)
        {
            if (bufferInfo[i].arrayBufferType != SMVBO_INDEX)
            {
                glBindBuffer(GL_ARRAY_BUFFER, bufferIndices[i]);
                glEnableVertexAttribArray(bufferInfo[i].attributeIndex);
                glBufferSubData(GL_ARRAY_BUFFER_ARB, 0, bufferInfo[i].size, bufferInfo[i].attribPointer);
                //if the buffer is not disabled, it won't show the the model
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            if (vboType == SIMMEDTK_VBO_DYNAMIC && bufferInfo[i].arrayBufferType == SMVBO_INDEX)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIndices[i]);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferInfo[i].size, bufferInfo[i].attribPointer);
                //if the buffer is not disabled, it won't show the the model
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
        }

        return true;
    }
    return false;
}

void VAO::draw() const
{
    glPushAttrib(GL_ENABLE_BIT);
    shader->enableShader();
    shader->updateGLSLMatwithOPENGL();
    enable();
    updateStreamData();
    glDrawElements(GL_TRIANGLES, bufferInfo[indexBufferLocation].nbrElements, GL_UNSIGNED_INT, bufferInfo[indexBufferLocation].attribPointer);
    disable();
    shader->disableShader();
    glPopAttrib();
}
VBOBufferEntryInfo::VBOBufferEntryInfo()
{
    shaderAttribLocation = -1;
    attributeIndex = 1;
    attribPointer = nullptr;
    nbrElements = 0;
    arrayBufferType = SMVBO_POS;
}
VAO::VAO( std::shared_ptr<ErrorLog> p_log, VBOType p_vboType, bool p_bindShaderObjects )
{
    this->log = p_log;
    renderingError = false;
    totalNbrBuffers = 0;
    vboType = p_vboType;
    VAOs[this->getUniqueId()->getId()] = safeDownCast<VAO>();
    indexBufferLocation = -1;
    bindShaderObjects = p_bindShaderObjects;
}

void VAO::setBufferData( VBOBufferType p_type, std::string p_ShaderAttribName, int p_nbrElements, void *p_ptr )
{
    bufferInfo[totalNbrBuffers].arrayBufferType = p_type;

    if ( p_type == SMVBO_POS ||
            p_type == SMVBO_NORMALS ||
            p_type == SMVBO_TANGENTS ||
            p_type == SMVBO_VEC3F )
    {
        bufferInfo[totalNbrBuffers].size = sizeof( core::Vec3d ) * p_nbrElements;
    }
    else if ( p_type == SMVBO_TEXTURECOORDS ||
              p_type == SMVBO_VEC2F )
    {
        bufferInfo[totalNbrBuffers].size = sizeof( TexCoord ) * p_nbrElements;
    }
    else if ( p_type == SMVBO_VEC4F )
    {
        bufferInfo[totalNbrBuffers].size = sizeof( float ) * 4 * p_nbrElements;
    }

    bufferInfo[totalNbrBuffers].attribPointer = p_ptr;
    bufferInfo[totalNbrBuffers].nbrElements = p_nbrElements;
    bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
    bufferInfo[totalNbrBuffers].shaderAttribName = p_ShaderAttribName;
    totalNbrBuffers++;
}
void VAO::setTriangleInfo( std::string p_ShaderAttribName, int p_nbrTriangles, void *p_ptr )
{
    bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_INDEX;
    bufferInfo[totalNbrBuffers].nbrElements = p_nbrTriangles * 3;
    bufferInfo[totalNbrBuffers].attribPointer = p_ptr;
    bufferInfo[totalNbrBuffers].size = sizeof( int ) * p_nbrTriangles * 3;
    bufferInfo[totalNbrBuffers].shaderAttribName = p_ShaderAttribName;
    totalNbrBuffers++;
}
bool VAO::setBufferDataFromMesh( Mesh *p_mesh, std::shared_ptr<Shader> p_shader, std::string p_POSITIONShaderName, std::string p_NORMALShaderName, std::string p_TEXTURECOORDShaderName, std::string p_TANGENTSName )
{
    if ( p_shader == nullptr )
    {
        shader = Shader::getShader( p_mesh->getRenderDetail()->shaders[0] );
    }
    else
    {
        shader = p_shader;
    }

    bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_POS;
    bufferInfo[totalNbrBuffers].size = sizeof( core::Vec3d ) * p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attribPointer = p_mesh->vertices.data();
    bufferInfo[totalNbrBuffers].nbrElements = p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
    bufferInfo[totalNbrBuffers].shaderAttribName = p_POSITIONShaderName;
    totalNbrBuffers++;

    bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_NORMALS;
    bufferInfo[totalNbrBuffers].size = sizeof( core::Vec3d ) * p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attribPointer = p_mesh->vertNormals;
    bufferInfo[totalNbrBuffers].nbrElements = p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
    bufferInfo[totalNbrBuffers].shaderAttribName = p_NORMALShaderName;
    totalNbrBuffers++;

    ///texture coord is for each vertex
    bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_TEXTURECOORDS;
    bufferInfo[totalNbrBuffers].size = sizeof( TexCoord ) * p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attribPointer = p_mesh->texCoord;
    bufferInfo[totalNbrBuffers].nbrElements = p_mesh->nbrVertices;
    bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
    bufferInfo[totalNbrBuffers].shaderAttribName = p_TEXTURECOORDShaderName;
    totalNbrBuffers++;

    if ( p_mesh->tangentChannel )
    {
        bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_TANGENTS;
        bufferInfo[totalNbrBuffers].size = sizeof( core::Vec3d ) * p_mesh->nbrVertices;
        bufferInfo[totalNbrBuffers].attribPointer = p_mesh->vertTangents;
        bufferInfo[totalNbrBuffers].nbrElements = p_mesh->nbrVertices;
        bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
        bufferInfo[totalNbrBuffers].shaderAttribName = p_TANGENTSName;
        totalNbrBuffers++;
    }

    bufferInfo[totalNbrBuffers].arrayBufferType = SMVBO_INDEX;
    bufferInfo[totalNbrBuffers].nbrElements = p_mesh->nbrTriangles * 3;
    bufferInfo[totalNbrBuffers].attribPointer = p_mesh->triangles;
    bufferInfo[totalNbrBuffers].size = sizeof( int ) * p_mesh->nbrTriangles * 3;
    bufferInfo[totalNbrBuffers].attributeIndex = totalNbrBuffers;
    totalNbrBuffers++;

    mesh = p_mesh;
    return true;
}
void VAO::initVAOs()
{
    for ( auto & x : VAOs )
    {
        x.second->initBuffers();
    }
}
std::shared_ptr<VAO> VAO::getVAO( std::shared_ptr<UnifiedId> p_shaderID )
{
    return VAOs[p_shaderID->getId()];
}
void VAO::enable() const
{
    glBindVertexArray( vaObject );
}
void VAO::disable() const
{
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
}
VAO::~VAO()
{
    glDeleteBuffers( totalNbrBuffers, bufferIndices );
    glDeleteVertexArrays( 1, &vaObject );
}

