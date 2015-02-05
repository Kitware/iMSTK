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

#include "smRendering/smVAO.h"
#include "smShader/smShader.h"
#include "smRendering/smViewer.h"

QHash<smInt, smVAO *>  smVAO::VAOs;

void smVAO::initBuffers(smDrawParam p_param)
{

    ///Create the Vertex Array Objects
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    ///Create Vertex Buffer Objects(VBOs)
    glGenBuffers(totalNbrBuffers, bufferIndices);
    assert(bufferIndices > 0);

    ///Initialize and file the VBOs
    for (smInt i = 0; i < totalNbrBuffers; i++)
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
smBool smVAO::updateStreamData()
{

    if (this->vboType == SIMMEDTK_VBO_STATIC)
    {
        return false;
    }

    else if (vboType == SIMMEDTK_VBO_NOINDICESCHANGE || vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        for (smInt i = 0; i < totalNbrBuffers; i++)
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
}

void smVAO::draw(smDrawParam p_params)
{

    float m[16];
    float m1[16];
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