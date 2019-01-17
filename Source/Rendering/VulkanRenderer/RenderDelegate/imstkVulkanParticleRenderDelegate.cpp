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

#include "imstkVulkanParticleRenderDelegate.h"

namespace imstk
{
VulkanParticleRenderDelegate::VulkanParticleRenderDelegate(std::shared_ptr<VisualModel> visualModel,
                                                           SceneObject::Type type,
                                                           VulkanMemoryManager& memoryManager)
{
    this->initialize(visualModel);

    auto geometry = std::static_pointer_cast<RenderParticles>(visualModel->getGeometry());

    m_numVertices = 4;
    m_numTriangles = 2;
    m_vertexSize = sizeof(VulkanBasicVertex);

    m_visualModel->getRenderMaterial()->m_isParticle = true;

    m_particles.reserve(geometry->m_maxNumParticles);
    m_particleIndices.reserve(geometry->m_maxNumParticles);
    m_particleDistances.reserve(geometry->m_maxNumParticles);
    this->initializeData(memoryManager, this->getVisualModel()->getRenderMaterial());

    this->updateVertexBuffer();
}

void
VulkanParticleRenderDelegate::updateVertexBuffer()
{
    auto vertices = (VulkanBasicVertex *)m_vertexBuffer->getVertexMemory();
    auto geometry = std::static_pointer_cast<RenderParticles>(m_visualModel->getGeometry());

    for (unsigned i = 0; i < m_numVertices; i++)
    {
        vertices[i].position = geometry->m_vertexPositions[i];
        vertices[i].uv = geometry->m_vertexUVs[i];
        vertices[i].normal = geometry->m_vertexNormals[i];
    }

    auto triangles = (std::array<uint32_t, 3> *)m_vertexBuffer->getIndexMemory();

    for (unsigned i = 0; i < m_numTriangles; i++)
    {
        triangles[i][0] = (uint32_t)geometry->m_triangles[i].x;
        triangles[i][1] = (uint32_t)geometry->m_triangles[i].y;
        triangles[i][2] = (uint32_t)geometry->m_triangles[i].z;
    }
}

void
VulkanParticleRenderDelegate::initializeData(VulkanMemoryManager& memoryManager, std::shared_ptr<RenderMaterial> material)
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
VulkanParticleRenderDelegate::update(const uint32_t frameIndex, std::shared_ptr<Camera> camera)
{
    unsigned int index = 0;

    auto particles = std::static_pointer_cast<RenderParticles>(m_visualModel->getGeometry());

    auto mat = this->getVisualModel()->getRenderMaterial();
    auto cameraPosition = glm::vec3(camera->getPosition()[0],
        camera->getPosition()[1],
        camera->getPosition()[2]);
    auto cameraUp = glm::vec3(camera->getViewUp()[0],
        camera->getViewUp()[1],
        camera->getViewUp()[2]);

    auto matColor = mat->getColor();

    this->sortParticles(particles->getParticles(), particles->getNumParticles(), cameraPosition);

    for (unsigned int i = 0; i < particles->getNumParticles(); i++)
    {
        auto particlePosition = glm::vec3(m_particles[i]->m_position[0],
            m_particles[i]->m_position[1],
            m_particles[i]->m_position[2]);

        // Point the particle to the camera
        glm::mat4 transformation;
        glm::mat4 billboardTransformation;
        this->generateBillboardMatrix(particlePosition, cameraPosition, cameraUp, billboardTransformation);
        transformation = glm::translate(transformation, particlePosition);
        transformation = transformation * billboardTransformation;
        transformation = glm::rotate(transformation, m_particles[i]->m_rotation, glm::vec3(0, 0, 1.0f));
        transformation = glm::scale(transformation,
            glm::vec3(particles->m_particleSize) * m_particles[i]->m_scale);
        m_particleVertexUniforms.transform[i] = transformation;

        m_particleFragmentUniforms.receivesShadows[i] = mat->getReceivesShadows() ? 1 : 0;
        m_particleFragmentUniforms.emissivity[i] = mat->getEmissivity();
        m_particleFragmentUniforms.roughness[i] = mat->getRoughness();
        m_particleFragmentUniforms.metalness[i] = mat->getMetalness();
        m_particleFragmentUniforms.color[i][0] = m_particles[i]->m_color.r;
        m_particleFragmentUniforms.color[i][1] = m_particles[i]->m_color.g;
        m_particleFragmentUniforms.color[i][2] = m_particles[i]->m_color.b;
        m_particleFragmentUniforms.color[i][3] = m_particles[i]->m_color.a;
    }

    m_vertexUniformBuffer->updateUniforms(sizeof(VulkanLocalParticleVertexUniforms),
        (void *)&m_particleVertexUniforms, frameIndex);
    m_fragmentUniformBuffer->updateUniforms(sizeof(VulkanLocalParticleFragmentUniforms),
        (void *)&m_particleFragmentUniforms, frameIndex);
}

void
VulkanParticleRenderDelegate::generateBillboardMatrix(const glm::vec3& objectPosition,
                                                      const glm::vec3& cameraPosition,
                                                      const glm::vec3& cameraUp,
                                                      glm::mat4& transformation)
{
    auto forwardVector = glm::normalize(cameraPosition - objectPosition);
    auto rightVector = glm::cross(cameraUp, forwardVector);
    auto upVector = glm::cross(forwardVector, rightVector);
    transformation[0][0] = rightVector[0];
    transformation[0][1] = rightVector[1];
    transformation[0][2] = rightVector[2];
    transformation[1][0] = upVector[0];
    transformation[1][1] = upVector[1];
    transformation[1][2] = upVector[2];
    transformation[2][0] = forwardVector[0];
    transformation[2][1] = forwardVector[1];
    transformation[2][2] = forwardVector[2];
}

void
VulkanParticleRenderDelegate::sortParticles(const std::vector<std::unique_ptr<RenderParticle>>& renderParticles,
                                            unsigned int numParticles,
                                            const glm::vec3& cameraPosition)
{
    m_particles.resize(numParticles);
    m_particleIndices.resize(numParticles);
    m_particleDistances.resize(numParticles);

    for (unsigned int i = 0; i < numParticles; i++)
    {
        float dx = cameraPosition.x - renderParticles[i]->m_position[0];
        float dy = cameraPosition.y - renderParticles[i]->m_position[1];
        float dz = cameraPosition.z - renderParticles[i]->m_position[2];

        // Avoid square root
        m_particleDistances[i] = (dx * dx) + (dy * dy) + (dz * dz);
        m_particleIndices[i] = i;
    }

    // Sort particles
    for (unsigned int i = 0; i < numParticles; i++)
    {
        for (unsigned int j = 0; j < numParticles; j++)
        {
            if (m_particleDistances[m_particleIndices[j]] <
                m_particleDistances[m_particleIndices[i]])
            {
                unsigned int temp = m_particleIndices[i];
                m_particleIndices[i] = m_particleIndices[j];
                m_particleIndices[j] = temp;
            }
        }
    }

    // Reorder particles
    for (unsigned int i = 0; i < numParticles; i++)
    {
        m_particles[i] = renderParticles[m_particleIndices[i]].get();
    }
}
}
