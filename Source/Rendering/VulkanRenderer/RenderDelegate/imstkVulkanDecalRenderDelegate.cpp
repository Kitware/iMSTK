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

#include "imstkVulkanDecalRenderDelegate.h"

namespace imstk
{
VulkanDecalRenderDelegate::VulkanDecalRenderDelegate(std::shared_ptr<DecalPool> decalPool, VulkanMemoryManager& memoryManager)
    : m_geometry(decalPool)
{
    m_numVertices = 8;
    m_numTriangles = 12;
    m_vertexSize = sizeof(VulkanBasicVertex);

    if (!m_geometry->getRenderMaterial())
    {
        auto material = std::make_shared<RenderMaterial>();
        m_geometry->setRenderMaterial(material);
    }
    m_geometry->getRenderMaterial()->m_isDecal = true;

    this->initializeData(memoryManager, m_geometry->getRenderMaterial());

    this->updateVertexBuffer();
}

void
VulkanDecalRenderDelegate::updateVertexBuffer()
{
    auto vertices = (VulkanBasicVertex *)m_vertexBuffer->mapVertices();

    for (unsigned i = 0; i < m_numVertices; i++)
    {
        vertices[i].position =
            m_geometry->m_vertexPositions[i];
    }

    m_vertexBuffer->unmapVertices();

    auto triangles = (std::array<uint32_t, 3> *)m_vertexBuffer->mapTriangles();

    for (unsigned i = 0; i < m_numTriangles; i++)
    {
        triangles[i][0] = (uint32_t)m_geometry->m_triangles[i].x;
        triangles[i][1] = (uint32_t)m_geometry->m_triangles[i].y;
        triangles[i][2] = (uint32_t)m_geometry->m_triangles[i].z;
    }
    m_vertexBuffer->unmapTriangles();
}

void
VulkanDecalRenderDelegate::initializeData(VulkanMemoryManager& memoryManager, std::shared_ptr<RenderMaterial> material)
{
    m_vertexUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalDecalVertexUniforms));
    m_fragmentUniformBuffer = std::make_shared<VulkanUniformBuffer>(memoryManager, (uint32_t)sizeof(VulkanLocalDecalFragmentUniforms));

    m_material = std::make_shared<VulkanMaterialDelegate>(m_vertexUniformBuffer,
        m_fragmentUniformBuffer,
        material,
        memoryManager);

    m_vertexBuffer = std::make_shared<VulkanVertexBuffer>(memoryManager, m_numVertices, m_vertexSize, m_numTriangles);
}

void
VulkanDecalRenderDelegate::update(std::shared_ptr<Camera> camera)
{
    unsigned int index = 0;

    auto eye = glm::tvec3<float>(camera->getPosition().x(), camera->getPosition().y(), camera->getPosition().z());
    auto center = glm::tvec3<float>(camera->getFocalPoint().x(), camera->getFocalPoint().y(), camera->getFocalPoint().z());
    auto up = glm::tvec3<float>(camera->getViewUp().x(), camera->getViewUp().y(), camera->getViewUp().z());
    auto viewMatrix = glm::lookAt(eye, center, up);

    for (auto decal : m_geometry->getDecals())
    {
        decal->updateDecal(viewMatrix);
        m_decalVertexUniforms.transform[index] = decal->m_transform;
        m_decalFragmentUniforms.inverse[index] = decal->m_inverse;
        index++;
    }

    m_vertexUniformBuffer->updateUniforms(sizeof(glm::mat4) * m_geometry->getMaxNumDecals(),
        (void *)&m_decalVertexUniforms);
    m_fragmentUniformBuffer->updateUniforms(sizeof(glm::mat4) * m_geometry->getMaxNumDecals(),
        (void *)&m_decalFragmentUniforms);
}

std::shared_ptr<Geometry>
VulkanDecalRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}
