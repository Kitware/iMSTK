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
VulkanLineMeshRenderDelegate::VulkanLineMeshRenderDelegate(std::shared_ptr<LineMesh> lineMesh, VulkanMemoryManager& memoryManager)
    : m_geometry(lineMesh)
{
    m_numVertices = (uint32_t)m_geometry->getNumVertices();
    m_numTriangles = (uint32_t)m_geometry->getNumLines();
    m_loadFactor = m_geometry->getLoadFactor();
    m_vertexSize = sizeof(VulkanBasicVertex);

    if (!m_geometry->getRenderMaterial())
    {
        m_geometry->setRenderMaterial(std::make_shared<RenderMaterial>());
    }

    m_geometry->getRenderMaterial()->m_isLineMesh = true;

    this->initializeData(memoryManager, m_geometry->getRenderMaterial());

    this->updateVertexBuffer();

    this->update(0);
}

void
VulkanLineMeshRenderDelegate::updateVertexBuffer()
{
    auto vertices = (VulkanBasicVertex *)m_vertexBuffer->getVertexMemory();

    auto colors = m_geometry->getVertexColors();

    auto vertexPositions = m_geometry->getVertexPositions(Geometry::DataType::PreTransform);
    for (unsigned i = 0; i < m_geometry->getNumVertices(); i++)
    {
        vertices[i].position = glm::vec3(
            vertexPositions[i][0],
            vertexPositions[i][1],
            vertexPositions[i][2]);

        if (colors.size() == m_geometry->getNumVertices())
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

    auto lines = (std::array<uint32_t, 2> *)m_vertexBuffer->getIndexMemory();

    m_vertexBuffer->setNumIndices((uint32_t)m_geometry->getNumLines() * 2);

    auto lineVertices = m_geometry->getLinesVertices();
    for (unsigned i = 0; i < m_geometry->getNumLines(); i++)
    {
        lines[i][0] = (uint32_t)lineVertices[i][0];
        lines[i][1] = (uint32_t)lineVertices[i][1];
    }
}

void
VulkanLineMeshRenderDelegate::update(const uint32_t frameIndex)
{
    this->updateUniforms(frameIndex);

    if (m_geometry->m_dataModified)
    {
        this->updateVertexBuffer();
    }
}

std::shared_ptr<Geometry>
VulkanLineMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}