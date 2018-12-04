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

#include "imstkVulkanLineMeshRenderDelegate.h"

namespace imstk
{
VulkanLineMeshRenderDelegate::VulkanLineMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel,
                                                           SceneObject::Type type,
                                                           VulkanMemoryManager& memoryManager)
{
    this->initialize(visualModel);

    auto geometry = std::static_pointer_cast<LineMesh>(visualModel->getGeometry());

    m_numVertices = (uint32_t)geometry->getNumVertices();
    m_numTriangles = (uint32_t)geometry->getNumLines();
    m_loadFactor = geometry->getLoadFactor();
    m_vertexSize = sizeof(VulkanBasicVertex);

    this->getVisualModel()->getRenderMaterial()->m_isLineMesh = true;

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
VulkanLineMeshRenderDelegate::updateVertexBuffer(const uint32_t frameIndex)
{
    int frame = 0;
    if (m_vertexBuffer->getMode() != VERTEX_BUFFER_STATIC)
    {
        frame = frameIndex;
    }

    auto vertices = (VulkanBasicVertex *)m_vertexBuffer->getVertexMemory(frame);
    auto geometry = std::static_pointer_cast<LineMesh>(m_visualModel->getGeometry());

    auto colors = geometry->getVertexColors();

    auto vertexPositions = geometry->getVertexPositions(Geometry::DataType::PreTransform);
    for (unsigned i = 0; i < geometry->getNumVertices(); i++)
    {
        vertices[i].position = glm::vec3(
            vertexPositions[i][0],
            vertexPositions[i][1],
            vertexPositions[i][2]);

        if (colors.size() == geometry->getNumVertices())
        {
            vertices[i].color = glm::vec3(
                colors[i].r,
                colors[i].g,
                colors[i].b);
        }
        else
        {
            vertices[i].color = glm::vec3(1, 1, 1);
        }
    }

    auto lines = (std::array<uint32_t, 2> *)m_vertexBuffer->getIndexMemory(frame);

    m_vertexBuffer->setNumIndices((uint32_t)geometry->getNumLines() * 2);

    auto lineVertices = geometry->getLinesVertices();
    for (unsigned i = 0; i < geometry->getNumLines(); i++)
    {
        lines[i][0] = (uint32_t)lineVertices[i][0];
        lines[i][1] = (uint32_t)lineVertices[i][1];
    }
}

void
VulkanLineMeshRenderDelegate::update(const uint32_t frameIndex)
{
    this->updateUniforms(frameIndex);

    auto geometry = std::static_pointer_cast<LineMesh>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        this->updateVertexBuffer(frameIndex);
    }
}
}