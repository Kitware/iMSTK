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

#include "VBO.h"

smVBO::smVBO( smErrorLog *p_log )
{
    this->log = p_log;
    renderingError = false;
}

smVBO::~smVBO()
{
    glDeleteBuffersARB( 1, &vboDataId );
    glDeleteBuffersARB( 1, &vboIndexId );
}

/// WARNING: This function takes arrays to Eigen 3d vectors,
/// it is not clear to me that the memory is aligned.
smVBOResult smVBO::updateVertices(const smVectorf &p_vectors,
                                  const smVectorf &p_normals,
                                  const smVectorf &p_textureCoords,
                                  size_t p_objectId)
{
    if (vboType == SIMMEDTK_VBO_STATIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    // Bind the vertices's VBO
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);

    smFloat *objectBufferPtr = reinterpret_cast<smFloat*>(glMapBufferARB(
        GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB)) + dataOffsetMap[p_objectId];
    if (objectBufferPtr == NULL)
    {
        log->addError("VBO could not map the buffer");
        renderingError = true;
        return SIMMEDTK_VBO_BUFFERPOINTERERROR;
    }

    // Copy vertices
    objectBufferPtr = std::copy(p_vectors.data(),
                                p_vectors.data()+p_vectors.size(),
                                objectBufferPtr);

    // Copy normals
    objectBufferPtr = std::copy(p_normals.data(),
                                p_normals.data()+p_normals.size(),
                                objectBufferPtr);

    // Copy texture coords
    if (p_textureCoords.size() > 0)
    {
        std::copy(p_textureCoords.data(),
                  p_textureCoords.data()+p_textureCoords.size(),
                  objectBufferPtr);
    }

    // Unmap the buffer
    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

smVBOResult smVBO::updateTriangleIndices(const smVector<size_t> &p_indices, size_t p_objectId)
{
    if ((vboType == SIMMEDTK_VBO_STATIC) | (vboType == SIMMEDTK_VBO_DYNAMIC))
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    // Bind the indices' VBO
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);

    smFloat *objectBufferPtr = reinterpret_cast<smFloat*>(glMapBufferARB(
        GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB)) + indexOffsetMap[p_objectId];
    if (objectBufferPtr == NULL)
    {
        log->addError("VBO could not map the buffer");
        renderingError = true;
        return SIMMEDTK_VBO_BUFFERPOINTERERROR;
    }

    // Copy indices
    std::copy(p_indices.data(),
              p_indices.data()+p_indices.size(),
              objectBufferPtr);

    // Unmap the buffer
    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

smVBOResult smVBO::drawElements(size_t p_objectId)
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
    glVertexPointer(3, smGLRealType, 0, reinterpret_cast<void*>(dataOffset));
    glNormalPointer(smGLRealType, 0, reinterpret_cast<void*>(dataOffset + nbrVertices * sizeof(smVec3d)));
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
    glIndexPointer(smGLUIntType, 0, reinterpret_cast<void*>(indexOffset));

    //render polygons
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_INDEX_ARRAY);

    if (!renderingError)
    {
        glDrawElements(GL_TRIANGLES, nbrTriangles * 3, smGLUIntType, reinterpret_cast<void*>(indexOffset));
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
smVBOResult smVBO::initStaticVertices(const smVectorf &p_vectors,
                                      const smVectorf &p_normals,
                                      const smVectorf &p_textureCoords,
                                      size_t p_objectId)
{
    if (vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    size_t dataOffset = dataOffsetMap[p_objectId];

    // Bind the vertices's VBO
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, p_vectors.size() * sizeof(float), p_vectors.data());
    dataOffset += p_vectors.size() * sizeof(float);

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, p_normals.size() * sizeof(float), p_normals.data());
    dataOffset += p_normals.size() * sizeof(float);

    if (p_textureCoords.size() > 0)
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, dataOffset, p_textureCoords.size() * sizeof(float), p_textureCoords.data());
    }

    // Unmap the buffer
    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

///init Triangle Indices for the very first time for static objects
///this function must be called when the indices are not changing
///SIMMEDTK_VBO_NOINDICESCHANGE
smVBOResult smVBO::initTriangleIndices(const smVector<size_t> &p_indices, size_t p_objectId)
{
    size_t indexOffset;

    if (vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        return SIMMEDTK_VBO_INVALIDOPERATION;
    }

    indexOffset = indexOffsetMap[p_objectId];
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
    glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexOffset, sizeof(smTriangle)*p_indices.size(), p_indices.data());

    // Unmap the buffer
    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    return SIMMEDTK_VBO_OK;
}

void smVBO::init( smVBOType p_vboType )
{
    smString error;
    glGenBuffersARB(1, &vboDataId);
    glGenBuffersARB(1, &vboIndexId);
    assert(vboDataId > 0);
    assert(vboIndexId > 0);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);

    if (p_vboType == SIMMEDTK_VBO_STATIC)
    {
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, SIMMEDTK_VBOBUFFER_DATASIZE, 0, GL_STATIC_DRAW);
    }
    else if (p_vboType == SIMMEDTK_VBO_DYNAMIC || p_vboType == SIMMEDTK_VBO_NOINDICESCHANGE)
    {
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, SIMMEDTK_VBOBUFFER_DATASIZE, 0, GL_STREAM_DRAW);
    }

    SM_CHECKERROR(log, error)
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);

    if (p_vboType == SIMMEDTK_VBO_STATIC || p_vboType == SIMMEDTK_VBO_NOINDICESCHANGE)
    {
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SIMMEDTK_VBOBUFFER_INDEXSIZE, 0, GL_STATIC_DRAW);
    }
    else if (p_vboType == SIMMEDTK_VBO_DYNAMIC)
    {
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SIMMEDTK_VBOBUFFER_INDEXSIZE, 0, GL_STREAM_DRAW);
    }

    SM_CHECKERROR(log, error);
    vboType = p_vboType;
    sizeOfDataBuffer = SIMMEDTK_VBOBUFFER_DATASIZE;
    sizeOfIndexBuffer = SIMMEDTK_VBOBUFFER_INDEXSIZE;
    currentDataOffset = 0;
    currentIndexOffset = 0;
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

smVBOResult smVBO::addVerticestoBuffer( const size_t p_nbrVertices, const size_t p_nbrTriangles, const size_t p_objectId )
{
    if ( sizeof( smVec3d )*p_nbrVertices + sizeof( smVec3d )*p_nbrVertices + sizeof( smTexCoord )*p_nbrVertices > sizeOfDataBuffer - currentDataOffset )
    {
        return SIMMEDTK_VBO_NODATAMEMORY;
    }

    if ( sizeof( smInt )*p_nbrTriangles * 3 > size_t(sizeOfIndexBuffer - currentIndexOffset))
    {
        return SIMMEDTK_VBO_NODATAMEMORY;
    }

    dataOffsetMap[p_objectId] = currentDataOffset;
    indexOffsetMap[p_objectId] = currentIndexOffset;
    numberofVertices[p_objectId] = p_nbrVertices;
    numberofTriangles[p_objectId] = p_nbrTriangles;
    ///add the vertices and normals and the texture coordinates
    currentDataOffset += sizeof( smVec3d ) * p_nbrVertices + sizeof( smVec3d ) * p_nbrVertices + sizeof( smTexCoord ) * p_nbrVertices;
    currentIndexOffset += p_nbrTriangles * sizeof( smTriangle );
    return SIMMEDTK_VBO_OK;
}
