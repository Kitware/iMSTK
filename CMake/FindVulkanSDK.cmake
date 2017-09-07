#-----------------------------------------------------------------------------
# Vulkan renderer
#-----------------------------------------------------------------------------
message(STATUS "Superbuild -   Vulkan SDK => ENABLING Vulkan renderer")
if(NOT DEFINED VulkanSDK_ROOT_DIR)
  set(VulkanSDK_ROOT_DIR "$ENV{VULKAN_SDK}" CACHE PATH "Path to Vulkan SDK install directory." FORCE)
endif()
if(NOT EXISTS ${VulkanSDK_ROOT_DIR})
  message(FATAL_ERROR "\nCan not support Vulkan renderer without Vulkan SDK.\nSet VulkanSDK_ROOT_DIR to Vulkan SDK installation directory.\n\n")
endif()

#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(VulkanSDK_INCLUDE_DIR
  NAMES
    vulkan/vulkan.h
  PATHS
    ${VulkanSDK_ROOT_DIR}/Include
    )
mark_as_advanced(VulkanSDK_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(VulkanSDK_LIBRARY
  NAMES
    vulkan-1
  )
mark_as_advanced(VulkanSDK_LIBRARY)

set(VulkanSDK_LIBRARIES ${VulkanSDK_LIBRARY})