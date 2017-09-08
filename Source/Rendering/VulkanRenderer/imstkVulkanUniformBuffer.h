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

struct VulkanLocalFragmentUniforms
{
    glm::mat4 transform;
};

struct VulkanLight
{
    glm::vec3 lightVector;
    float lightAngle;
    glm::vec3 lightColor;
    float lightIntensity;
};

struct VulkanGlobalVertexUniforms
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec4 cameraPosition;
    VulkanLight lights[16];
};

struct VulkanGlobalFragmentUniforms
{
    VulkanLight lights[16];
};


class VulkanUniformBuffer : public VulkanBuffer
{
public:
    VulkanUniformBuffer(VulkanMemoryManager& memoryManager, uint32_t uniformSize);

    void updateUniforms(uint32_t uniformSize, void * uniformData);

    ~VulkanUniformBuffer() = default;

    ///
    /// \brief Binds the vertex buffer to memory
    ///
    void Bind();

protected:
    friend class VulkanRenderer;
    friend class VulkanMaterialDelegate;

    VkBuffer * getUniformBuffer();

    void * mapUniforms();
    void unmapUniforms();

    VkBuffer m_uniformBuffer;
    VkDeviceMemory m_uniformMemory;

    VkDevice m_renderDevice;
    uint32_t m_bufferMemoryIndex;

    VkDeviceSize m_uniformBufferSize;

    static const uint32_t maxBufferSize = 1024 * 1024;

private:
    VulkanUniformBuffer() = delete;
};
}

#endif