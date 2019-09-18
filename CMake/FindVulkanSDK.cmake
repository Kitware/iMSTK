#-----------------------------------------------------------------------------
# Vulkan renderer
#-----------------------------------------------------------------------------
if(NOT DEFINED VulkanSDK_ROOT_DIR OR NOT EXISTS ${VulkanSDK_ROOT_DIR})
  set(VulkanSDK_ROOT_DIR "$ENV{VULKAN_SDK}" CACHE PATH "Path to Vulkan SDK install directory." FORCE)
endif()
if(NOT EXISTS ${VulkanSDK_ROOT_DIR})
  message(FATAL_ERROR "\nCannot support Vulkan renderer without Vulkan SDK.\nSet VulkanSDK_ROOT_DIR to Vulkan SDK installation directory.\n\n")
endif()

include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for VulkanSDK
#-----------------------------------------------------------------------------

imstk_find_header(VulkanSDK vulkan/vulkan.h)
imstk_find_libary(VulkanSDK vulkan-1)
imstk_find_package(VulkanSDK)

#message(STATUS "VulkanSDK include : ${VULKANSDK_INCLUDE_DIRS}")
#message(STATUS "VulkanSDK libraries : ${VULKANSDK_LIBRARIES}")
