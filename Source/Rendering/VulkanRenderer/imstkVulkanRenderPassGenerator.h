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

#pragma once

#include "vulkan/vulkan.h"
#include "imstkVulkanUtilities.h"

namespace imstk
{
class VulkanRenderPassGenerator
{
public:
    static void generateDepthRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateOpaqueRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateDecalRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateParticleRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateShadowRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateGUIRenderPass(
        VkDevice&              device,
        VkRenderPass&          renderPass,
        VkSampleCountFlagBits& samples,
        const uint32_t         numViews);

    static void generateRenderPassMultiviewCreateInfo(
        VkRenderPassMultiviewCreateInfo& multiviewInfo,
        const uint32_t&                  viewMask,
        const uint32_t&                  correlationMask);
};
}
