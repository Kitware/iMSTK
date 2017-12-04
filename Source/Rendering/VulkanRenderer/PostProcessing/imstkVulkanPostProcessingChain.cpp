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

#include "imstkVulkanPostProcessingChain.h"

#include "imstkVulkanRenderer.h"

namespace imstk
{
VulkanPostProcessingChain::VulkanPostProcessingChain(VulkanRenderer * renderer)
{
    // Subsurface scattering pass
    // The buffer indices are hardcoded because it's before the accumulation composition pass
    if (m_sss)
    {
        int sssSamples = 5;

        auto sssHorizontalBlurPass = std::make_shared<VulkanPostProcess>(renderer, 0);
        sssHorizontalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[0][0]);
        sssHorizontalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_depthImageView[0],
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        sssHorizontalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_normalImageView);
        sssHorizontalBlurPass->m_framebuffer->setColor(&renderer->m_HDRImageView[2][0], VK_FORMAT_R16G16B16A16_SFLOAT);
        sssHorizontalBlurPass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/sss_frag.spv");
        sssHorizontalBlurPass->m_pushConstantData[0] = 1.0;
        sssHorizontalBlurPass->m_pushConstantData[1] = 0.0;
        sssHorizontalBlurPass->m_pushConstantData[2] = renderer->m_fov;
        sssHorizontalBlurPass->m_pushConstantData[3] = 3.0;
        sssHorizontalBlurPass->m_pushConstantData[4] = renderer->m_nearPlane;
        sssHorizontalBlurPass->m_pushConstantData[5] = renderer->m_farPlane;
        sssHorizontalBlurPass->m_pushConstantData[6] = sssSamples;
        VulkanPostProcessingChain::calculateBlurValues(sssSamples,
            &sssHorizontalBlurPass->m_pushConstantData[7],
            renderer->m_nearPlane);
        VulkanPostProcessingChain::calculateBlurValues(sssSamples,
            &sssHorizontalBlurPass->m_pushConstantData[17],
            renderer->m_farPlane);
        m_postProcesses.push_back(sssHorizontalBlurPass);

        auto sssVerticalBlurPass = std::make_shared<VulkanPostProcess>(renderer, 0);
        sssVerticalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[2][0]);
        sssVerticalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_depthImageView[0],
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        sssVerticalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_normalImageView);
        sssVerticalBlurPass->m_framebuffer->setColor(&renderer->m_HDRImageView[0][0], VK_FORMAT_R16G16B16A16_SFLOAT);
        sssVerticalBlurPass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/sss_frag.spv");
        sssVerticalBlurPass->m_pushConstantData[0] = 0.0;
        sssVerticalBlurPass->m_pushConstantData[1] = 1.0;
        sssVerticalBlurPass->m_pushConstantData[2] = renderer->m_fov;
        sssVerticalBlurPass->m_pushConstantData[3] = 3.0;
        sssVerticalBlurPass->m_pushConstantData[4] = renderer->m_nearPlane;
        sssVerticalBlurPass->m_pushConstantData[5] = renderer->m_farPlane;
        sssVerticalBlurPass->m_pushConstantData[6] = sssSamples;
        VulkanPostProcessingChain::calculateBlurValues(sssSamples,
            &sssVerticalBlurPass->m_pushConstantData[7],
            renderer->m_nearPlane);
        VulkanPostProcessingChain::calculateBlurValues(sssSamples,
            &sssVerticalBlurPass->m_pushConstantData[17],
            renderer->m_farPlane);
        m_postProcesses.push_back(sssVerticalBlurPass);
    }

    // Accumulation composition pass
    auto accumulationCompositePass = std::make_shared<VulkanPostProcess>(renderer);
    accumulationCompositePass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[0][0]);
    accumulationCompositePass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[1][0]);
    accumulationCompositePass->m_framebuffer->setColor(&renderer->m_HDRImageView[2][0], VK_FORMAT_R16G16B16A16_SFLOAT);
    accumulationCompositePass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/composite_frag.spv");
    m_postProcesses.push_back(accumulationCompositePass);

    // Bloom pass
    if (m_bloom)
    {
        int level = 1;
        int bloomSamples = 5;

        auto bloomThresholdPass = std::make_shared<VulkanPostProcess>(renderer, level);
        bloomThresholdPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[m_lastOutput][0]);
        bloomThresholdPass->m_framebuffer->setColor(&renderer->m_HDRImageView[m_lastInput][level], VK_FORMAT_R16G16B16A16_SFLOAT);
        bloomThresholdPass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/bloom_threshold_frag.spv");
        m_postProcesses.push_back(bloomThresholdPass);

        auto bloomHorizontalBlurPass = std::make_shared<VulkanPostProcess>(renderer, level);
        bloomHorizontalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[m_lastInput][level]);
        bloomHorizontalBlurPass->m_framebuffer->setColor(&renderer->m_HDRImageView[m_lastOutput][level], VK_FORMAT_R16G16B16A16_SFLOAT);
        bloomHorizontalBlurPass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/blur_horizontal_frag.spv");
        bloomHorizontalBlurPass->m_pushConstantData[0] = std::max(renderer->m_width >> level, 1u);
        bloomHorizontalBlurPass->m_pushConstantData[1] = std::max(renderer->m_height >> level, 1u);
        bloomHorizontalBlurPass->m_pushConstantData[2] = bloomSamples;
        VulkanPostProcessingChain::calculateBlurValuesLinear(bloomSamples,
            &bloomHorizontalBlurPass->m_pushConstantData[3],
            &bloomHorizontalBlurPass->m_pushConstantData[13]);
        m_postProcesses.push_back(bloomHorizontalBlurPass);

        auto bloomVerticalBlurPass = std::make_shared<VulkanPostProcess>(renderer, level);
        bloomVerticalBlurPass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[m_lastOutput][level]);
        bloomVerticalBlurPass->m_framebuffer->setColor(&renderer->m_HDRImageView[m_lastInput][level], VK_FORMAT_R16G16B16A16_SFLOAT);
        bloomVerticalBlurPass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/blur_vertical_frag.spv");
        bloomVerticalBlurPass->m_pushConstantData[0] = std::max(renderer->m_width >> level, 1u);
        bloomVerticalBlurPass->m_pushConstantData[1] = std::max(renderer->m_height >> level, 1u);
        bloomVerticalBlurPass->m_pushConstantData[2] = bloomSamples;
        VulkanPostProcessingChain::calculateBlurValuesLinear(bloomSamples,
            &bloomVerticalBlurPass->m_pushConstantData[3],
            &bloomVerticalBlurPass->m_pushConstantData[13]);
        m_postProcesses.push_back(bloomVerticalBlurPass);

        auto bloomCompositePass = std::make_shared<VulkanPostProcess>(renderer);
        bloomCompositePass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[m_lastOutput][0]);
        bloomCompositePass->addInputImage(&renderer->m_HDRImageSampler, &renderer->m_HDRImageView[m_lastInput][level]);
        bloomCompositePass->m_framebuffer->setColor(&renderer->m_HDRImageView[m_lastInput][0], VK_FORMAT_R16G16B16A16_SFLOAT);
        bloomCompositePass->initialize(renderer, "./Shaders/VulkanShaders/PostProcessing/composite_frag.spv");
        m_postProcesses.push_back(bloomCompositePass);
        this->incrementBufferNumbers();
    }
}

std::vector<std::shared_ptr<VulkanPostProcess>>&
VulkanPostProcessingChain::getPostProcesses()
{
    return m_postProcesses;
}

void
VulkanPostProcessingChain::incrementBufferNumbers()
{
    m_lastInput = (m_lastInput + 1) % 3;
    m_lastOutput = (m_lastOutput + 1) % 3;
}

void
VulkanPostProcessingChain::calculateBlurValuesLinear(int samples, float * values, float * offsets)
{
    std::vector<float> intermediateValues(samples * 2 - 1);
    float total = 0;

    // Calculate normal distribution
    for (int i = 0; i < intermediateValues.size(); i++)
    {
        float x = (i / (float)intermediateValues.size()) * 3;
        intermediateValues[i] = 1.0 / std::sqrt(2 * PI) * std::pow(NLOG_E, -(x * x) / 2);
        total += intermediateValues[i] * 2;
    }

    // Normalize
    for (int i = 0; i < intermediateValues.size(); i++)
    {
        intermediateValues[i] /= total;
    }

    values[0] = intermediateValues[0];
    offsets[0] = 0.0;

    // Linear sampling optimization
    for (int i = 1; i < samples; i++)
    {
        values[i] = intermediateValues[2 * i - 1] + intermediateValues[2 * i];
        offsets[i] = (((2 * i - 1) * intermediateValues[2 * i - 1]) +
                      ((2 * i) * intermediateValues[2 * i])) / values[i];
    }
}

void
VulkanPostProcessingChain::calculateBlurValues(int samples, float * values, float stdDev)
{
    float total = 0;

    // Calculate normal distribution
    for (int i = 0; i < samples; i++)
    {
        values[i] = 1.0 / std::sqrt(2 * PI * stdDev * stdDev) * std::pow(NLOG_E, -(i * i) / (2 * stdDev * stdDev));

        if (i == 0)
        {
            total += values[i];
        }
        else
        {
            total += values[i] * 2;
        }
    }

    // Normalize
    for (int i = 0; i < samples; i++)
    {
        values[i] /= total;
    }
}
}