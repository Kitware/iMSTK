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

#ifndef SMVBO_H
#define SMVBO_H

// STL includes
#include <unordered_map>
#include <cassert>

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Mesh/Mesh.h"
#include "ConfigRendering.h"
#include "GLUtils.h"
#include "Core/Utils.h"
#include "Core/Vector.h"

/// \brief VBO for rendering
class smVBO: public CoreClass
{
private:
    /// \brief offsets for each mesh
    size_t currentDataOffset;
    size_t currentIndexOffset;
    size_t sizeOfDataBuffer;
    size_t sizeOfIndexBuffer;
    /// \brief  VBO type
    smVBOType vboType;
    /// \brief  vertices, normals, tangets data buffer
    GLuint vboDataId;
    /// \brief index data
    GLuint vboIndexId;
    /// \brief  data offset keeps offset for each vertex
    std::unordered_map<size_t, size_t> dataOffsetMap;
    /// \brief  index maps
    std::unordered_map<size_t, size_t> indexOffsetMap;
    /// \brief  total number of vertices
    std::unordered_map<size_t, size_t> numberofVertices;
    /// \brief number of triangles
    std::unordered_map<size_t, size_t> numberofTriangles;
    /// \brief  error log
    ErrorLog *log;
    /// \brief  rendering error
    bool renderingError;

public:
    /// \brief  constructor. gets error log or NULL
    smVBO(ErrorLog *p_log);

    /// \brief  init with given VBO type
    void init(smVBOType p_vboType);

    /// \brief  add vertices to the data buffer
    smVBOResult addVerticestoBuffer(const size_t p_nbrVertices,
                                    const size_t p_nbrTriangles,
                                    const size_t p_objectId);

    /// \brief update vertex data buffer
    smVBOResult updateVertices(const core::Vectorf &p_vectors,
                                  const core::Vectorf &p_normals,
                                  const core::Vectorf &p_textureCoords,
                                  size_t p_objectId);

    /// \brief update  triangle index
    smVBOResult updateTriangleIndices(const Vector<size_t> &p_indices, size_t p_objectId);

    /// \brief draw elements in VBO
    smVBOResult drawElements(size_t p_objectId);

    /// \brief update the static vertices initially
    smVBOResult initStaticVertices(const core::Vectorf &p_vectors,
                                  const core::Vectorf &p_normals,
                                  const core::Vectorf &p_textureCoords,
                                  size_t p_objectId);

    /// \brief update the static triangle indices initially
    smVBOResult initTriangleIndices(const Vector<size_t> &p_indices, size_t p_objectId);

    /// \brief deletion of the VBO buffers
    ~smVBO();

};

#endif
