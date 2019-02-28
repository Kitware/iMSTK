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
/// \brief Vulkan texture implementation
///
class VulkanTextureDelegate
{
public:
    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    /// \param anisotropyAmount Amount of anisotropic filtering to apply
    ///
    VulkanTextureDelegate(VulkanMemoryManager& memoryManager,
                          std::shared_ptr<Texture> texture,
                          float anisotropyAmount = 0.0f);

    ///
    /// \brief File reader for a 2D texture
    /// \param memoryManager Memory manager to control allocation
    ///
    void loadTexture(VulkanMemoryManager& memoryManager);

    ///
    /// \brief File reader for a cubemap texture (supports mipmap reading)
    /// \param memoryManager Memory manager to control allocation
    ///
    void loadCubemapTexture(VulkanMemoryManager& memoryManager);

    ///
    /// \brief Upload 2D texture to the GPU
    /// \param memoryManager Memory manager to control allocation
    /// Also generates mipmaps if necessary
    ///
    void uploadTexture(VulkanMemoryManager& memoryManager);

    ///
    /// \brief Upload cubemap texture to the GPU
    /// \param memoryManager Memory manager to control allocation
    ///
    void uploadCubemapTexture(VulkanMemoryManager& memoryManager);

    ///
    /// \brief Change image layout
    /// \param commandBuffer Command buffer used to change the layout
    /// \param image Image to change layout
    /// \param sourceLayout Current layout
    /// \param destinationLayout New layout
    /// \param sourceFlags Current access flags
    /// \param destinationFlags New access flags
    /// \param range Image range
    ///
    static void changeImageLayout(VkCommandBuffer& commandBuffer,
                                  VkImage& image,
                                  VkImageLayout layout1,
                                  VkImageLayout layout2,
                                  VkAccessFlags sourceFlags,
                                  VkAccessFlags destinationFlags,
                                  VkImageSubresourceRange range);

    ///
    /// \brief Generate mipmaps
    /// \param commandBuffer Command buffer used to change the layout
    ///
    void generateMipmaps(VkCommandBuffer& commandBuffer);

    ///
    /// \brief Get number of channels for format
    /// \param format Image format
    /// \returns Number of channels in image (e.g., RGBA = 4 channels)
    ///
    unsigned int getNumChannels(const VkFormat& format);

    ///
    /// \brief Get stride for format
    /// \param format Image format
    /// \returns Stride required for format (i.e., number of bytes between pixels)
    ///
    VkDeviceSize getStride(const VkFormat& format);

    ///
    /// \brief Helper function to find the closest block size
    /// \param imageExtent Dimensions of an image
    /// \param blockExtent Dimensions of a block
    /// \returns Modified dimensions to be a multiple of the block size
    /// For compressed images, the dimensions of each mip level must be a
    /// multiple of the block size.
    ///
    const glm::ivec3 getDimensionsAlignedToBlockSize(
        const glm::ivec3& imageSize,
        const glm::ivec3& blockSize);

    ///
    /// \brief Destroy the image
    /// \param device Device on which the image is located (e.g., GPU)
    ///
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
    Texture::FileType m_fileType;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    bool m_isDataFormatted = false;
    unsigned int m_mipLevels = 0;
    bool m_loadMipMaps = false;
    unsigned int m_arrayLayers = 1;
    float anisotropyAmount = 1.0f;

    unsigned int m_width = 0;
    unsigned int m_height = 0;
    unsigned int m_channels = 0;

    bool m_isCompressed = false;
    unsigned char * m_data;
    gli::texture_cube m_cubemap; ///> Only used for cubemaps
    gli::texture m_compressedTexture; ///> only used for DDS files
    bool m_isCubemap = false;

    VkDeviceSize m_imageOffsetAlignment;
    VulkanMemoryManager * m_memoryManager;
};
}

#endif