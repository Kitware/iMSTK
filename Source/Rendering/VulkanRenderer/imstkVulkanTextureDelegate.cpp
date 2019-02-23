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

#include "imstkVulkanTextureDelegate.h"

namespace imstk
{
VulkanTextureDelegate::VulkanTextureDelegate(
    VulkanMemoryManager& memoryManager,
    std::shared_ptr<Texture> texture,
    float anisotropyAmount)
{
    m_path = texture->getPath();
    m_type = texture->getType();
    m_fileType = texture->getFileType();

    // Load textures and get texture information
    if ((m_type == Texture::Type::IRRADIANCE_CUBEMAP)
        || (m_type == Texture::Type::RADIANCE_CUBEMAP))
    {
        m_arrayLayers = 6;
        this->loadCubemapTexture(memoryManager);
        m_imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        m_isCubemap = true;
    }
    else
    {
        m_arrayLayers = 1;
        this->loadTexture(memoryManager);
        m_imageInfo.flags = 0;
    }

    // Determine number of mipmaps
    if (m_mipLevels < 1)
    {
        if (!texture->getMipmapsEnabled()
            || (m_type == Texture::Type::BRDF_LUT)
            || (m_path == "noise"))
        {
            m_mipLevels = 1;
        }
        else
        {
            m_mipLevels = std::log2(std::max(m_width, m_height)) + 1;
        }
    }

    m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_imageOffsetAlignment = this->getStride(m_format);

    m_imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    m_imageInfo.pNext = nullptr;
    m_imageInfo.format = m_format;
    m_imageInfo.imageType = VK_IMAGE_TYPE_2D;
    m_imageInfo.extent = { m_width, m_height, 1 };
    m_imageInfo.mipLevels = m_mipLevels;
    m_imageInfo.arrayLayers = m_arrayLayers;
    m_imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    m_imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    m_imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    m_imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    m_imageInfo.queueFamilyIndexCount = 1;
    m_imageInfo.pQueueFamilyIndices = &memoryManager.m_queueFamilyIndex;
    m_imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    m_image = memoryManager.requestImage(memoryManager.m_device, m_imageInfo, VulkanMemoryType::TEXTURE);

    m_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_range.baseMipLevel = 0;
    m_range.levelCount = m_mipLevels;
    m_range.baseArrayLayer = 0;
    m_range.layerCount = m_arrayLayers;

    if (m_isCubemap)
    {
        this->uploadCubemapTexture(memoryManager);
    }
    else
    {
        this->uploadTexture(memoryManager);
    }

    VkComponentMapping mapping;
    mapping.r = VK_COMPONENT_SWIZZLE_R;
    mapping.g = VK_COMPONENT_SWIZZLE_G;
    mapping.b = VK_COMPONENT_SWIZZLE_B;
    mapping.a = VK_COMPONENT_SWIZZLE_A;

    VkImageViewCreateInfo imageViewInfo;
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext = nullptr;
    imageViewInfo.flags = 0;
    imageViewInfo.image = *m_image->getImage();

    if (!m_isCubemap)
    {
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else
    {
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }

    imageViewInfo.format = m_imageInfo.format;
    imageViewInfo.components = mapping;
    imageViewInfo.subresourceRange = m_range;

    vkCreateImageView(memoryManager.m_device, &imageViewInfo, nullptr, &m_imageView);

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Trilinear interpolation
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0;
    samplerInfo.anisotropyEnable = anisotropyAmount == 0.0f ? VK_FALSE : VK_TRUE; // TODO:: add option to enable
    samplerInfo.maxAnisotropy = anisotropyAmount;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = m_mipLevels - 1;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(memoryManager.m_device, &samplerInfo, nullptr, &m_sampler);
}

void
VulkanTextureDelegate::loadTexture(VulkanMemoryManager& memoryManager)
{
    if (m_path == "")
    {
        auto data = new std::vector<unsigned char>(4);
        (*data)[0] = '\255';
        (*data)[1] = '\255';
        (*data)[2] = '\255';
        (*data)[3] = '\255';
        m_width = 1;
        m_height = 1;
        m_channels = 1;
        m_data = &(*data)[0];
        m_format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else if (m_path == "noise")
    {
        auto data = new std::vector<unsigned char>(128 * 128 * 4);
        m_width = 128;
        m_height = 128;
        m_channels = 4;
        for (int x = 0; x < 128; x++)
        {
            for (int y = 0; y < 128; y++)
            {
                for (int z = 0; z < 4; z++)
                {
                    int seed = x * m_width * 4 + y * 4 + z;
                    (*data)[seed] = glm::linearRand(0, 255);
                }
            }
        }

        m_data = &(*data)[0];
        m_format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        switch (m_fileType)
        {
        case Texture::FileType::DDS:
        {
            m_compressedTexture = gli::load(m_path);
            m_compressedTexture = gli::flip(m_compressedTexture);
            m_format = (VkFormat)(m_compressedTexture.format());
            m_isDataFormatted = true;

            // Convert to SRGB for linear color space conversion
            if (m_type == Texture::Type::DIFFUSE)
            {
                switch(m_format)
                {
                case VK_FORMAT_B8G8R8A8_UNORM:
                    m_format = VK_FORMAT_B8G8R8A8_SRGB;
                    break;
                case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                    m_format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
                    break;
                case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                    m_format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
                    break;
                case VK_FORMAT_BC2_UNORM_BLOCK:
                    m_format = VK_FORMAT_BC2_SRGB_BLOCK;
                    break;
                case VK_FORMAT_BC3_UNORM_BLOCK:
                    m_format = VK_FORMAT_BC3_SRGB_BLOCK;
                    break;
                default:
                    LOG(WARNING) << "Texture format is not supported";
                }
            }

            m_channels = this->getNumChannels(m_format);
            m_data = (unsigned char *)m_compressedTexture.data();

            m_width = m_compressedTexture.extent().x;
            m_height = m_compressedTexture.extent().y;
            m_mipLevels = (uint32_t)m_compressedTexture.levels();     // Load mip levels
            m_loadMipMaps = m_mipLevels != 1 ? true : false;
            m_isCompressed = gli::is_compressed(m_compressedTexture.format());
        }
        break;
        case Texture::FileType::PNG:
        case Texture::FileType::JPG:
        case Texture::FileType::BMP:
        {
            auto readerGenerator = vtkSmartPointer<vtkImageReader2Factory>::New();
            auto reader = readerGenerator->CreateImageReader2(m_path.c_str());

            reader->SetFileName(m_path.c_str());
            reader->Update();

            auto data = reader->GetOutput();
            m_width = data->GetDimensions()[0];
            m_height = data->GetDimensions()[1];
            m_channels = reader->GetNumberOfScalarComponents();
            m_data = (unsigned char *)data->GetScalarPointer();

            // Format determines optimizations
            switch (m_type)
            {
            case Texture::Type::DIFFUSE:
                m_format = VK_FORMAT_B8G8R8A8_SRGB;
                break;
            case Texture::Type::AMBIENT_OCCLUSION:
            case Texture::Type::METALNESS:
            case Texture::Type::ROUGHNESS:
                m_format = VK_FORMAT_R8_UNORM;
                break;
            default:
                m_format = VK_FORMAT_B8G8R8A8_UNORM;
            }
        }
        break;
        default:
            LOG(WARNING) << "Unknown file type";
        }
    }
}

void
VulkanTextureDelegate::loadCubemapTexture(VulkanMemoryManager& memoryManager)
{
    if (m_path != "")
    {
        m_cubemap = gli::texture_cube(gli::load(m_path));

        m_width = m_cubemap.extent().x;
        m_height = m_cubemap.extent().y;
        m_mipLevels = (uint32_t)m_cubemap.levels();
        m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else
    {
        m_cubemap = gli::texture_cube(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, gli::extent2d(1,1), 1);
        m_width = 1;
        m_height = 1;
        m_mipLevels = 1;
        m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
}

void
VulkanTextureDelegate::uploadTexture(VulkanMemoryManager& memoryManager)
{
    uint32_t imageSize;
    if (m_isCompressed || m_loadMipMaps)
    {
        imageSize = (uint32_t)m_compressedTexture.size();
    }
    else
    {
        imageSize = m_width * m_height * m_imageOffsetAlignment;
    }

    // Staging image
    VkBufferCreateInfo stagingBufferInfo;
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.pNext = nullptr;
    stagingBufferInfo.flags = 0;
    stagingBufferInfo.size = imageSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    stagingBufferInfo.queueFamilyIndexCount = 0;
    stagingBufferInfo.pQueueFamilyIndices = nullptr;

    m_stagingBuffer = memoryManager.requestBuffer(memoryManager.m_device,
        stagingBufferInfo,
        VulkanMemoryType::STAGING_TEXTURE,
        m_imageOffsetAlignment);

    auto imageEditData = (unsigned char*)m_stagingBuffer->getMemoryData(memoryManager.m_device);

    unsigned int y_offset = 0;
    unsigned int totalChannels = this->getNumChannels(m_format);
    unsigned int colorChannels = std::min(std::min(m_channels, 3u), totalChannels);

    if (!m_isDataFormatted)
    {
        for (unsigned int y = 0; y < m_height; y++)
        {
            y_offset = y * m_width;
            for (unsigned int x = 0; x < m_width; x++)
            {
                // Fill in image data
                for (unsigned int z = 0; z < colorChannels; z++)
                {
                    imageEditData[totalChannels * (y_offset + x) + z] =
                        m_data[m_channels * (y_offset + x) + (colorChannels - z - 1)];
                }

                // Fill in the rest of the memory
                memset(&imageEditData[totalChannels * (y_offset + x) + colorChannels],
                    (unsigned char)255,
                    (totalChannels - colorChannels) * sizeof(unsigned char));

                // For alpha channel
                if (m_channels == 4)
                {
                    imageEditData[4 * (y_offset + x) + 3] = m_data[m_channels * (y_offset + x) + 3];
                }
            }
        }
    }
    else
    {
        memcpy(&imageEditData[0], m_compressedTexture.data(), imageSize);
    }

    // Start transfer commands
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(*memoryManager.m_transferCommandBuffer, &commandBufferBeginInfo);

    VkImageSubresourceLayers layersDestination;
    layersDestination.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    layersDestination.mipLevel = 0;
    layersDestination.baseArrayLayer = 0;
    layersDestination.layerCount = 1;

    VkBufferImageCopy copyInfo;
    copyInfo.bufferOffset = m_stagingBuffer->getOffset();
    copyInfo.bufferRowLength = m_width;
    copyInfo.bufferImageHeight = m_height;
    copyInfo.imageSubresource = layersDestination;
    copyInfo.imageOffset = { 0, 0, 0 };
    copyInfo.imageExtent = { m_width, m_height, 1 };

    if (m_isCompressed || m_loadMipMaps)
    {
        std::vector<VkBufferImageCopy> copyInfos(m_mipLevels);
        VkDeviceSize currentOffset = m_stagingBuffer->getOffset();
        glm::ivec3 blockSize = gli::block_extent(m_compressedTexture.format());

        for (uint32_t i = 0; i < m_mipLevels; i++)
        {
            glm::ivec3 dimensions = m_compressedTexture.extent(i);

            if (m_isCompressed)
            {
                dimensions = this->getDimensionsAlignedToBlockSize(
                    m_compressedTexture.extent(i),
                    blockSize);
            }

            copyInfos[i] = copyInfo;
            copyInfos[i].bufferRowLength = dimensions.x;
            copyInfos[i].bufferImageHeight = dimensions.y;
            copyInfos[i].imageSubresource.mipLevel = i;
            copyInfos[i].bufferOffset = currentOffset;
            copyInfos[i].imageExtent = { (uint32_t)dimensions.x, (uint32_t)dimensions.y, 1 };
            currentOffset += m_compressedTexture.size(i);
        }

        this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, m_range);
        vkCmdCopyBufferToImage(*memoryManager.m_transferCommandBuffer, *m_stagingBuffer->getBuffer(),
            *m_image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, &copyInfos[0]);
        this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, m_range);
    }
    else
    {
        this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, m_range);
        vkCmdCopyBufferToImage(*memoryManager.m_transferCommandBuffer, *m_stagingBuffer->getBuffer(),
            *m_image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        if (m_mipLevels != 1)
        {
            this->generateMipmaps(*memoryManager.m_transferCommandBuffer);
        }

        this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, m_range);
    }

    vkEndCommandBuffer(*memoryManager.m_transferCommandBuffer);

    VkCommandBuffer commandBuffers[] = { *memoryManager.m_transferCommandBuffer };

    VkPipelineStageFlags stageWaitFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[1];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount = 0;
    submitInfo[0].pWaitSemaphores = nullptr;
    submitInfo[0].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = commandBuffers;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = nullptr;

    vkQueueSubmit(*memoryManager.m_transferQueue, 1, submitInfo, nullptr);
    vkDeviceWaitIdle(memoryManager.m_device);
}

void
VulkanTextureDelegate::uploadCubemapTexture(VulkanMemoryManager& memoryManager)
{
    uint32_t imageSize = (uint32_t)m_cubemap.size();

    // Staging image
    VkBufferCreateInfo stagingBufferInfo;
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.pNext = nullptr;
    stagingBufferInfo.flags = 0;
    stagingBufferInfo.size = imageSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    stagingBufferInfo.queueFamilyIndexCount = 0;
    stagingBufferInfo.pQueueFamilyIndices = nullptr;

    m_stagingBuffer = memoryManager.requestBuffer(memoryManager.m_device,
        stagingBufferInfo,
        VulkanMemoryType::STAGING_TEXTURE,
        m_imageOffsetAlignment);

    memcpy(m_stagingBuffer->getMemoryData(memoryManager.m_device), m_cubemap.data(), m_cubemap.size());

    m_stagingBuffer->unmapMemory(memoryManager.m_device);

    // Start transfer commands
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(*memoryManager.m_transferCommandBuffer, &commandBufferBeginInfo);

    std::vector<VkBufferImageCopy> copyInfos(m_mipLevels * m_arrayLayers);

    unsigned int currentOffset = 0;
    for (unsigned int layer = 0; layer < m_arrayLayers; layer++)
    {
        for (unsigned int level = 0; level < m_mipLevels; level++)
        {
            VkImageSubresourceLayers layersDestination;
            layersDestination.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            layersDestination.mipLevel = level;
            layersDestination.baseArrayLayer = layer;
            layersDestination.layerCount = 1;

            unsigned int currentRegion = layer * m_mipLevels + level;

            copyInfos[currentRegion].bufferOffset = currentOffset + m_stagingBuffer->getOffset();
            copyInfos[currentRegion].bufferRowLength = m_cubemap[layer][level].extent().x;
            copyInfos[currentRegion].bufferImageHeight = m_cubemap[layer][level].extent().y;
            copyInfos[currentRegion].imageSubresource = layersDestination;
            copyInfos[currentRegion].imageOffset = { 0, 0, 0 };
            copyInfos[currentRegion].imageExtent = {
                (uint32_t)m_cubemap[layer][level].extent().x,
                (uint32_t)m_cubemap[layer][level].extent().y,
                1
            };
            currentOffset += (unsigned int)m_cubemap[layer][level].size();
        }
    }

    this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
        VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, m_range);

    vkCmdCopyBufferToImage(*memoryManager.m_transferCommandBuffer, *m_stagingBuffer->getBuffer(),
        *m_image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)copyInfos.size(), &copyInfos[0]);

    this->changeImageLayout(*memoryManager.m_transferCommandBuffer, *m_image->getImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, m_range);

    vkEndCommandBuffer(*memoryManager.m_transferCommandBuffer);

    VkCommandBuffer commandBuffers[] = { *memoryManager.m_transferCommandBuffer };

    VkPipelineStageFlags stageWaitFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[1];
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].pNext = nullptr;
    submitInfo[0].waitSemaphoreCount = 0;
    submitInfo[0].pWaitSemaphores = nullptr;
    submitInfo[0].pWaitDstStageMask = &stageWaitFlags;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = commandBuffers;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = nullptr;

    vkQueueSubmit(*memoryManager.m_transferQueue, 1, submitInfo, nullptr);
    vkDeviceWaitIdle(memoryManager.m_device);
}

void
VulkanTextureDelegate::changeImageLayout(VkCommandBuffer& commandBuffer,
                                         VkImage& image,
                                         VkImageLayout layout1,
                                         VkImageLayout layout2,
                                         VkAccessFlags sourceFlags,
                                         VkAccessFlags destinationFlags,
                                         VkImageSubresourceRange range)
{
    VkImageMemoryBarrier layoutChange;
    layoutChange.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    layoutChange.pNext = nullptr;
    layoutChange.srcAccessMask = sourceFlags;
    layoutChange.dstAccessMask = destinationFlags;
    layoutChange.oldLayout = layout1;
    layoutChange.newLayout = layout2;
    layoutChange.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutChange.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutChange.image = image;
    layoutChange.subresourceRange = range;

    auto sourceStageFlags = VK_PIPELINE_STAGE_HOST_BIT;
    auto destinationStageFlags = VK_PIPELINE_STAGE_HOST_BIT;

    if (sourceFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT))
    {
        sourceStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (sourceFlags & (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT))
    {
        sourceStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (destinationFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT))
    {
        destinationStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (destinationFlags & (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT))
    {
        destinationStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,
        sourceStageFlags,
        destinationStageFlags,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &layoutChange);
}

void
VulkanTextureDelegate::generateMipmaps(VkCommandBuffer& commandBuffer)
{
    for (uint32_t i = 0; i < m_mipLevels - 1; i++)
    {
        VkImageSubresourceLayers sourceLayers;
        sourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        sourceLayers.mipLevel = i;
        sourceLayers.baseArrayLayer = 0;
        sourceLayers.layerCount = 1;

        VkImageSubresourceLayers destinationLayers;
        destinationLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        destinationLayers.mipLevel = i + 1;
        destinationLayers.baseArrayLayer = 0;
        destinationLayers.layerCount = 1;

        VkOffset3D sourceOffsets[2];
        sourceOffsets[0].x = 0;
        sourceOffsets[0].y = 0;
        sourceOffsets[0].z = 0;

        sourceOffsets[1].x = m_width / (1 << i);
        sourceOffsets[1].y = m_height / (1 << i);
        sourceOffsets[1].z = 1;

        VkOffset3D destinationOffsets[2];
        destinationOffsets[0].x = 0;
        destinationOffsets[0].y = 0;
        destinationOffsets[0].z = 0;

        destinationOffsets[1].x = m_width / (1 << (i + 1));
        destinationOffsets[1].y = m_height / (1 << (i + 1));
        destinationOffsets[1].z = 1;

        VkImageBlit mipFormat;
        mipFormat.srcSubresource = sourceLayers;
        mipFormat.srcOffsets[0] = sourceOffsets[0];
        mipFormat.srcOffsets[1] = sourceOffsets[1];
        mipFormat.dstSubresource = destinationLayers;
        mipFormat.dstOffsets[0] = destinationOffsets[0];
        mipFormat.dstOffsets[1] = destinationOffsets[1];

        VkImageSubresourceRange mipHighRange = m_range;
        mipHighRange.baseMipLevel = i;
        mipHighRange.levelCount = 1;

        this->changeImageLayout(commandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, mipHighRange);

        vkCmdBlitImage(commandBuffer,
            *m_image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *m_image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &mipFormat, VK_FILTER_LINEAR);

        this->changeImageLayout(commandBuffer, *m_image->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, mipHighRange);
    }
}

unsigned int
VulkanTextureDelegate::getNumChannels(const VkFormat& format)
{
    unsigned int numChannels = 0;

    switch(format)
    {
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        numChannels = 4;
        break;
    case VK_FORMAT_R8_UNORM:
        numChannels = 1;
        break;
    default:
        LOG(WARNING) << "Stride unknown";
    }

    return numChannels;
}

VkDeviceSize
VulkanTextureDelegate::getStride(const VkFormat& format)
{
    VkDeviceSize stride = 4;

    switch(format)
    {
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
        stride = 4;
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        stride = 16;
        break;
    case VK_FORMAT_R8_UNORM:
        stride = 1;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
        stride = 16; // The compressed formats work in blocks
        break;
    default:
        LOG(WARNING) << "Stride unknown";
    }

    return stride;
}

const glm::ivec3
VulkanTextureDelegate::getDimensionsAlignedToBlockSize(
    const glm::ivec3& imageSize,
    const glm::ivec3& blockSize)
{
    glm::ivec3 dimensions;

    for (unsigned int i = 0; i < 3; i++)
    {
        auto remainder = imageSize[i] % blockSize[i];
        auto division = imageSize[i] / blockSize[i];
        dimensions[i] = division * blockSize[i];

        if (remainder != 0)
        {
            dimensions[i] += blockSize[i];
        }
    }
    return dimensions;
}

void
VulkanTextureDelegate::clear(VkDevice * device)
{
    vkDestroyImageView(*device, m_imageView, nullptr);
    vkDestroySampler(*device, m_sampler, nullptr);
}
}