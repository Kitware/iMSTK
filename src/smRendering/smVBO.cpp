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

#include "smRendering/smVBO.h"

smVBOResult smVBO::updateVertices(smVec3<smFloat> *p_vectors, smVec3<smFloat> *p_normals,
                                  smTexCoord *p_textureCoords, smInt p_objectId)
{

    smInt nbrVertices;
    smFloat *baseBufferPtr;
    smFloat *objectBufferPtr;

    if (vboType == SIMMEDTK_VBO_STATIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    nbrVertices = numberofVertices[p_objectId];
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);
    baseBufferPtr = (smFloat*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
    objectBufferPtr = (smFloat*)((smChar*)baseBufferPtr + dataOffsetMap[p_objectId]);

    if (objectBufferPtr == NULL)
    {
        log->addError("VBO could not map the buffer");
        renderingError = true;
        return SIMMEDTK_VBO_BUFFERPOINTERERROR;
    }

    //copy the buffers for vertices, normals and texture respectively
    memcpy(objectBufferPtr, p_vectors, nbrVertices * sizeof(smVec3<smFloat>));
    objectBufferPtr = (smFloat*)((smChar*)objectBufferPtr + nbrVertices * sizeof(smVec3<smFloat>));
    memcpy(objectBufferPtr, p_normals, nbrVertices * sizeof(smVec3<smFloat>));
    objectBufferPtr = (smFloat*)((smChar*)objectBufferPtr + nbrVertices * sizeof(smVec3<smFloat>));

    if (p_textureCoords != NULL)
    {
        memcpy(objectBufferPtr, p_textureCoords, nbrVertices * sizeof(smTexCoord));
    }

    //unmap the buffer
    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

smVBOResult smVBO::updateTriangleIndices(smInt *p_indices, smInt p_objectId)
{
    smInt nbrTriangles;
    smFloat *baseBufferPtr;
    smFloat *objectBufferPtr;

    if (vboType == SIMMEDTK_VBO_STATIC | vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    nbrTriangles = numberofTriangles[p_objectId];
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
    baseBufferPtr = (smFloat*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
    objectBufferPtr = (smFloat*)((smChar*)baseBufferPtr + indexOffsetMap[p_objectId]);

    if (objectBufferPtr == NULL)
    {
        log->addError("VBO could not map the buffer");
        renderingError = true;
        return SIMMEDTK_VBO_BUFFERPOINTERERROR;
    }

    memcpy(objectBufferPtr, p_indices, nbrTriangles * sizeof(smTriangle));
    //unmap the buffer
    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

smVBOResult smVBO::drawElements(smInt p_objectId)
{

    smInt dataOffset;
    smInt nbrVertices;
    smInt indexOffset;
    smInt nbrTriangles;

    nbrVertices = numberofVertices[p_objectId];
    nbrTriangles = numberofTriangles[p_objectId];
    dataOffset = dataOffsetMap[p_objectId];
    indexOffset = indexOffsetMap[p_objectId];
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);
    glVertexPointer(3, smGLRealType, 0, (void*)dataOffset);
    glNormalPointer(smGLRealType, 0, (void*)(dataOffset + nbrVertices * sizeof(smVec3<smFloat>)));
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
    glIndexPointer(smGLUIntType, 0, (void*)indexOffset);

    //render polygons
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_INDEX_ARRAY);

    if (!renderingError)
    {
        glDrawElements(GL_TRIANGLES, nbrTriangles * 3, smGLUIntType, (void*)indexOffset);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);

    //return back to original buffer
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    return SIMMEDTK_VBO_OK;
}

///Static Binding of the buffers. It is mandatory to call this function
/// this function must be called when the binding is SIMMEDTK_VBO_STATIC
smVBOResult smVBO::initStaticVertices(smVec3<smFloat> *p_vectors,
                                      smVec3<smFloat> *p_normals,
                                      smTexCoord *p_textureCoords,
                                      smInt p_objectId)
{

    smInt nbrVertices;
    smFloat *baseBufferPtr;
    smFloat *objectBufferPtr;
    smInt dataOffset;

    if (vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    dataOffset = dataOffsetMap[p_objectId];
    nbrVertices = numberofVertices[p_objectId];
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, nbrVertices * sizeof(smVec3<smFloat>), p_vectors);
    dataOffset += nbrVertices * sizeof(smVec3<smFloat>);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, nbrVertices * sizeof(smVec3<smFloat>), p_normals);
    dataOffset += nbrVertices * sizeof(smVec3<smFloat>);

    if (p_textureCoords != NULL)
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, nbrVertices * sizeof(smVec3<smFloat>), p_textureCoords);
    }

    //unmap the buffer
    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

///init Triangle Indices for the very first time for static objects
///this function must be called when the indices are not changing
///SIMMEDTK_VBO_NOINDICESCHANGE
smVBOResult smVBO::initTriangleIndices(smInt *p_indices, smInt p_objectId)
{

    smInt nbrTriangles;
    smFloat *baseBufferPtr;
    smFloat *objectBufferPtr;
    smInt indexOffset;

    if (vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    nbrTriangles = numberofTriangles[p_objectId];
    indexOffset = indexOffsetMap[p_objectId];
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
    glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexOffset, sizeof(smTriangle)*nbrTriangles, p_indices);

    //unmap the buffer
    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}