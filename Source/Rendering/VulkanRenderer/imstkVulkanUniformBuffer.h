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

#ifndef imstkVulkanUniformBuffer_h
#define imstkVulkanUniformBuffer_h

#include "imstkVulkanBuffer.h"
#include "imstkVulkanMemoryManager.h"

#include "vulkan/vulkan.h"

#include "glm/glm.hpp"

#include <vector>

namespace imstk
{
struct VulkanLocalVertexUniforms
{
    glm::mat4 transform;
};

struct VulkanLocalDecalVertexUniforms
{
    glm::mat4 transforms[128];
};

struct VulkanLocalParticleVertexUniforms
{
    glm::mat4 transform[128];
};

struct VulkanLocalFragmentUniforms
{
    glm::mat4 transform;
    glm::vec4 color;
    unsigned int receivesShadows;
    float emissivity;
    float roughness;
    float metalness;
    glm::vec4 debugColor;
};

struct VulkanLocalDecalFragmentUniforms
{
    glm::mat4 inverses[128];
    glm::vec4 color;
    unsigned int receivesShadows;
    float emissivity;
    float roughness;
    float metalness;
};

struct VulkanLocalParticleFragmentUniforms
{
    glm::vec4 color[128];
    unsigned int receivesShadows[128];
    float emissivity[128];
    float roughness[128];
    float metalness[128];
};

struct VulkanLight
{
    glm::vec4 position; // 3 position
    glm::vec4 color; // 3 color, 1 intensity
    glm::vec4 direction; // 3 direction, 1 angle
    glm::ivec4 state; // 1 type, 1 shadow map index
};

struct VulkanGlobalVertexUniforms
{
    glm::mat4 projectionMatrices[2];
    glm::mat4 viewMatrices[2];
    glm::vec4 cameraPositions[2];
    VulkanLight lights[16];
};

struct VulkanGlobalFragmentUniforms
{
    glm::mat4 inverseViewMatrices[2];
    glm::mat4 inverseProjectionMatrices[2];
    glm::vec4 resolution; // 2 resolution, 1 shadow map resolution
    VulkanLight lights[16];
    glm::mat4 lightMatrices[16];
};


class VulkanUniformBuffer : public VulkanBuffer
{
public:
    VulkanUniformBuffer(VulkanMemoryManager& memoryManager, uint32_t uniformSize);

    void updateUniforms(uint32_t uniformSize, void * uniformData, uint32_t frameIndex);

    ~VulkanUniformBuffer() = default;

protected:
    friend class VulkanRenderer;
    friend class VulkanMaterialDelegate;

    VulkanInternalBuffer * getUniformBuffer();

    void * getUniformMemory();

    VulkanInternalBuffer * m_uniformBuffer;

    VkDevice m_renderDevice;
    uint32_t m_bufferMemoryIndex;

    VkDeviceSize m_uniformBufferSize;

    static const uint32_t maxBufferSize = 1024 * 1024;

private:
    VulkanUniformBuffer() = delete;
};
}

#endif