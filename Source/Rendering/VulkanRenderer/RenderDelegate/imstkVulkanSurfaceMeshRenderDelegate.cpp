/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkVulkanSurfaceMeshRenderDelegate.h"

namespace imstk
{
VulkanSurfaceMeshRenderDelegate::VulkanSurfaceMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel,
                                                                 SceneObject::Type            type,
                                                                 VulkanMemoryManager&         memoryManager)
{
    this->initialize(visualModel);

    auto geometry = std::static_pointer_cast<SurfaceMesh>(visualModel->getGeometry());

    m_numVertices  = (uint32_t)geometry->getNumVertices();
    m_numTriangles = (uint32_t)geometry->getNumTriangles();
    m_loadFactor   = geometry->getLoadFactor();
    m_vertexSize   = sizeof(VulkanBasicVertex);
    m_modified.resize(memoryManager.m_buffering, true);

    if (!this->getVisualModel()->getRenderMaterial())
    {
        this->getVisualModel()->setRenderMaterial(std::make_shared<RenderMaterial>());
    }

    if (type == SceneObject::Type::FEMDeformable || type == SceneObject::Type::Pbd)
    {
        this->initializeData(memoryManager, this->getVisualModel()->getRenderMaterial(), VulkanVertexBufferMode::VERTEX_BUFFER_DYNAMIC);

        for (uint32_t i = 0; i < memoryManager.m_buffering; i++)
        {
            this->updateVertexBuffer(i);
            this->update(i);
        }
    }
    else
    {
        this->initializeData(memoryManager, this->getVisualModel()->getRenderMaterial(), VulkanVertexBufferMode::VERTEX_BUFFER_STATIC);

        this->updateVertexBuffer(0);
        this->update(0);
    }
}

void
VulkanSurfaceMeshRenderDelegate::updateVertexBuffer(uint32_t frameIndex)
{
    int  frame    = 0;
    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    if (m_vertexBuffer->getMode() == VERTEX_BUFFER_STATIC)
    {
        if (!geometry->m_dataModified)
        {
            return;
        }
        m_vertexBuffer->setModified();
    }
    else
    {
        frame = frameIndex;
    }

    auto vertices = (VulkanBasicVertex*)m_vertexBuffer->getVertexMemory(frame);

    auto                      normals  = geometry->getVertexNormals();
    auto                      tangents = geometry->getVertexTangents();
    const StdVectorOfVectorf* UVs;

    if (geometry->getDefaultTCoords() != "")
    {
        UVs = geometry->getPointDataArray(geometry->getDefaultTCoords());
    }
    else
    {
        UVs = nullptr;
    }

    auto vertexPositions = geometry->getVertexPositions(Geometry::DataType::PreTransform);
    for (unsigned i = 0; i < geometry->getNumVertices(); i++)
    {
        vertices[i].position = glm::vec3(
            vertexPositions[i][0],
            vertexPositions[i][1],
            vertexPositions[i][2]);

        if (normals.size() == geometry->getNumVertices())
        {
            vertices[i].normal = glm::vec3(
                normals[i][0],
                normals[i][1],
                normals[i][2]);
        }

        if (UVs && UVs->size() == geometry->getNumVertices())
        {
            if (tangents.size() == geometry->getNumVertices())
            {
                vertices[i].tangent = glm::vec3(
                    tangents[i][0],
                    tangents[i][1],
                    tangents[i][2]);
            }

            vertices[i].uv = glm::vec2(
                (*UVs)[i][0],
                (*UVs)[i][1]);
        }
    }

    auto triangles = (std::array<uint32_t, 3>*)m_vertexBuffer->getIndexMemory(frame);

    m_vertexBuffer->setNumIndices((uint32_t)geometry->getNumTriangles() * 3);
    for (unsigned i = 0; i < geometry->getNumTriangles(); i++)
    {
        triangles[i][0] = (uint32_t)geometry->getTrianglesVertices()[i][0];
        triangles[i][1] = (uint32_t)geometry->getTrianglesVertices()[i][1];
        triangles[i][2] = (uint32_t)geometry->getTrianglesVertices()[i][2];
    }
}

void
VulkanSurfaceMeshRenderDelegate::update(const uint32_t frameIndex)
{
    this->updateUniforms(frameIndex);

    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        geometry->computeVertexNormals();
        std::fill(m_modified.begin(), m_modified.end(), true);
        geometry->m_dataModified = false;
    }

    if (m_modified[frameIndex])
    {
        this->updateVertexBuffer(frameIndex);
        m_modified[frameIndex] = false;
    }
}
}