#-----------------------------------------------------------------------------
# Vulkan renderer
#-----------------------------------------------------------------------------

include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for VulkanSDK
#-----------------------------------------------------------------------------
set(VulkanSDK_LIB_DIR Lib)

imstk_find_header(VulkanSDK vulkan/vulkan.h)
imstk_find_libary(VulkanSDK vulkan-1 " ")#Use same library for debug)
imstk_find_package(VulkanSDK)

message(STATUS "VulkanSDK include : ${VULKANSDK_INCLUDE_DIRS}")
message(STATUS "VulkanSDK libraries : ${VULKANSDK_LIBRARIES}")
