/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */
 
#ifndef SMVBO_H
#define SMVBO_H

#include <QHash>
#include <GL/glew.h>
#include <GL/glut.h>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smConfigRendering.h"
#include "smUtilities/smVec3.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smUtils.h"
#include "assert.h"
/// \brief VBO for rendering
class smVBO: public smCoreClass
{
private:
    /// \brief offsets for each mesh
    smInt currentDataOffset;
    smInt currentIndexOffset;
    smInt sizeOfDataBuffer;
    smInt sizeOfIndexBuffer;
    /// \brief  VBO type
    smVBOType vboType;
    /// \brief  vertices, normals, tangets data buffer
    GLuint vboDataId;
    /// \brief index data
    GLuint vboIndexId;
    /// \brief  data offset keeps offset for each vertex
    QHash<smInt, smInt> dataOffsetMap;
    /// \brief  index maps
    QHash<smInt, smInt> indexOffsetMap;
    /// \brief  total number of vertices
    QHash<smInt, smInt> numberofVertices;
    /// \brief number of triangles
    QHash<smInt, smInt> numberofTriangles;
    /// \brief  error log
    smErrorLog *log;
    /// \brief  rendering error
    smBool renderingError;

public:
    /// \brief  constructor. gets error log or NULL
    smVBO(smErrorLog *p_log)
    {
        this->log = p_log;
        renderingError = false;
    }

    /// \brief  init with given VBO type
    void init(smVBOType p_vboType)
    {
        smChar error[200];
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

        SM_CHECKERROR(log, error)
        vboType = p_vboType;
        sizeOfDataBuffer = SIMMEDTK_VBOBUFFER_DATASIZE;
        sizeOfIndexBuffer = SIMMEDTK_VBOBUFFER_INDEXSIZE;
        currentDataOffset = 0;
        currentIndexOffset = 0;
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    /// \brief  add vertices to the data buffer
    smVBOResult addVerticestoBuffer(smInt p_nbrVertices, smInt p_nbrTriangles, smInt p_objectId)
    {
        if (sizeof(smVec3<smFloat>)*p_nbrVertices + sizeof(smVec3<smFloat>)*p_nbrVertices + sizeof(smTexCoord)*p_nbrVertices > sizeOfDataBuffer - currentDataOffset)
        {
            return SIMMEDTK_VBO_NODATAMEMORY;
        }

        if (sizeof(smInt)*p_nbrTriangles * 3 > sizeOfIndexBuffer - currentIndexOffset)
        {
            return SIMMEDTK_VBO_NODATAMEMORY;
        }

        dataOffsetMap[p_objectId] = currentDataOffset;
        indexOffsetMap[p_objectId] = currentIndexOffset;
        numberofVertices[p_objectId] = p_nbrVertices;
        numberofTriangles[p_objectId] = p_nbrTriangles;
        ///add the vertices and normals and the texture coordinates
        currentDataOffset += sizeof(smVec3<smFloat>) * p_nbrVertices + sizeof(smVec3<smFloat>) * p_nbrVertices + sizeof(smTexCoord) * p_nbrVertices;
        currentIndexOffset += p_nbrTriangles * sizeof(smTriangle);
        return SIMMEDTK_VBO_OK;
    }
    /// \brief update vertex data buffer
    smVBOResult updateVertices(smVec3<smFloat> *p_vectors, smVec3<smFloat> *p_normals, smTexCoord *p_textureCoords, smInt p_objectId);
    /// \brief update  triangle index
    smVBOResult updateTriangleIndices(smInt *p_indices, smInt p_objectId);
    /// \brief draw elements in VBO
    smVBOResult drawElements(smInt p_objectId);

    /// \brief update the static vertices initially
    smVBOResult initStaticVertices(smVec3<smFloat> *p_vectors, smVec3<smFloat> *p_normals, smTexCoord *p_textureCoords, smInt p_objectId);

    /// \brief update the static triangle indices initially
    smVBOResult initTriangleIndices(smInt *p_indices, smInt p_objectId);

    /// \brief deletion of the VBO buffers
    ~smVBO()
    {
        glDeleteBuffersARB(1, &vboDataId);
        glDeleteBuffersARB(1, &vboIndexId);
    }

};

#endif
