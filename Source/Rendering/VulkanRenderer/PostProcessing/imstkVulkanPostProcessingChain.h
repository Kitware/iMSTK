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
#include "glm/glm.hpp"

#include "imstkVulkanPostProcess.h"

#include <memory>
#include <vector>

namespace imstk
{
class VulkanRenderer;

class VulkanPostProcessingChain
{
public:
    ///
    /// \brief Constructor
    ///
    VulkanPostProcessingChain(VulkanRenderer * renderer);

    std::vector<std::shared_ptr<VulkanPostProcess>>& getPostProcesses();

protected:
    friend class VulkanRenderer;

    bool m_bloom = true; ///< bloom pass
    bool m_sss = true; ///< subsurface scattering pass

    std::vector<std::shared_ptr<VulkanPostProcess>> m_postProcesses;
    void incrementBufferNumbers();
    static void calculateBlurValuesLinear(int samples, float * values, float * offsets);
    static void calculateBlurValues(int samples, float * values, float stdDev = 0.0);

    unsigned int m_lastOutput = 2; ///< 2 by default because of accumulation composition
    unsigned int m_lastInput = 0; ///< 0 by default because of accumulation composition
};
}

