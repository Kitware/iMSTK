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

#ifndef imstkVulkanTexture_h
#define imstkVulkanTexture_h

#include "imstkTexture.h"
#include "imstkVulkanMemoryManager.h"

#include "vulkan/vulkan.h"

#include "gli/gli.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"

#include <vtkImageReader2.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageReader2Factory.h>

#include <string>
#include <memory>
#include <vector>

namespace imstk
{
///
/// \class VulkanTexture
///
/// \brief Vulkan texture implementation.
///
class VulkanTextureDelegate
{
public:
    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    ///
    VulkanTextureDelegate(VulkanMemoryManager& memoryManager,
                          std::shared_ptr<Texture> texture,
                          float anisotropyAmount = 0.0f);

    void loadTexture(VulkanMemoryManager& memoryManager);
    void loadCubemapTexture(VulkanMemoryManager& memoryManager);
    void uploadTexture(VulkanMemoryManager& memoryManager);
    void uploadCubemapTexture(VulkanMemoryManager& memoryManager);
    static void changeImageLayout(VkCommandBuffer& commandBuffer,
                                  VkImage& image,
                                  VkImageLayout layout1,
                                  VkImageLayout layout2,
                                  VkAccessFlags sourceFlags,
                                  VkAccessFlags destinationFlags,
                                  VkImageSubresourceRange range);

    void generateMipmaps(VkCommandBuffer& commandBuffer);
    void clear(VkDevice * device);

protected:
    friend class VulkanMaterialDelegate;
    friend class VulkanPostProcessingChain;
    friend class VulkanRenderer;

    VulkanInternalImage * m_image;

    VkImageView m_imageView;
    VkSampler m_sampler;
    VkImageLayout m_layout;
    VkImageCreateInfo m_imageInfo;

    VkImageSubresourceRange m_range;

    VulkanInternalBuffer * m_stagingBuffer;

    std::string m_path;
    Texture::Type m_type;
    unsigned int m_mipLevels = 0;
    unsigned int m_arrayLayers = 1;
    float anisotropyAmount = 1.0f;

    unsigned int m_width = 0;
    unsigned int m_height = 0;
    unsigned int m_channels = 0;

    unsigned char * m_data;
    gli::texture_cube m_cubemap; ///> Only used for cubemaps
    bool m_isCubemap = false;

    VkDeviceSize m_imageOffsetAlignment;
    VulkanMemoryManager * m_memoryManager;
};
}

#endif