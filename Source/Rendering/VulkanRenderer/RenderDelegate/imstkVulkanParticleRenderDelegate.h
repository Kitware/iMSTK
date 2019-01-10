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

#ifndef imstkVulkanParticleRenderDelegate_h
#define imstkVulkanParticleRenderDelegate_h

#include "imstkRenderParticleEmitter.h"
#include "imstkCamera.h"
#include "imstkVulkanRenderDelegate.h"

namespace imstk
{
class VulkanParticleRenderDelegate : public VulkanRenderDelegate {
public:

    ///
    /// \brief Default destructor
    ///
    ~VulkanParticleRenderDelegate() = default;

    ///
    /// \brief Default constructor
    ///
    VulkanParticleRenderDelegate(std::shared_ptr<VisualModel> visualModel,
                                 SceneObject::Type type,
                                 VulkanMemoryManager& memoryManager);

    ///
    /// \brief Update render geometry
    ///
    void update(uint32_t frameIndex, std::shared_ptr<Camera> camera);

    ///
    /// \brief Fill vertex buffer
    ///
    void updateVertexBuffer();

    ///
    /// \brief Initialize data
    ///
    void initializeData(VulkanMemoryManager& memoryManager,
                        std::shared_ptr<RenderMaterial> material = nullptr);


protected:
    VulkanLocalParticleVertexUniforms m_particleVertexUniforms;
    VulkanLocalParticleFragmentUniforms m_particleFragmentUniforms;

    ///
    /// \brief Generate billboard matrix
    ///
    void generateBillboardMatrix(const glm::vec3& objectPosition,
                                 const glm::vec3& cameraPosition,
                                 const glm::vec3& cameraUp,
                                 glm::mat4& transformation);

    ///
    /// \brief Sort particles
    ///
    void sortParticles(const std::vector<std::unique_ptr<RenderParticle>>& renderParticles,
                       unsigned int numParticles,
                       const glm::vec3& cameraPosition);

    std::vector<RenderParticle*> m_particles;
    std::vector<unsigned int> m_particleIndices;
    std::vector<float> m_particleDistances;
};
}

#endif
