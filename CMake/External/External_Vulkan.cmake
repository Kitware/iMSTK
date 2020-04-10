#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------

if(NOT DEFINED VulkanSDK_ROOT_DIR OR NOT EXISTS ${VulkanSDK_ROOT_DIR})
  set(VulkanSDK_ROOT_DIR "$ENV{VULKAN_SDK}" CACHE PATH "Path to Vulkan SDK install directory." FORCE)
endif()
if(NOT EXISTS ${VulkanSDK_ROOT_DIR})
  message(FATAL_ERROR "\nCannot support Vulkan renderer without Vulkan SDK.\nSet VulkanSDK_ROOT_DIR to Vulkan SDK installation directory.\n\n")
endif()

# Header Directories
set(vulkan_headers_dest ${CMAKE_INSTALL_PREFIX}/include/)
set(vulkan_headers_dir ${VulkanSDK_ROOT_DIR}/Include/)
# Library Directories
set(vulkan_libs_dest ${CMAKE_INSTALL_PREFIX}/lib/)
set(vulkan_libs_dir ${VulkanSDK_ROOT_DIR}/Lib)
# Binary Directory
set(vulkan_bin_dest ${CMAKE_INSTALL_PREFIX}/bin/)
set(vulkan_bin_dir ${VulkanSDK_ROOT_DIR}/Bin)

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Vulkan
  SOURCE_DIR ${VulkanSDK_ROOT_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${vulkan_headers_dir} ${vulkan_headers_dest}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${vulkan_libs_dir} ${vulkan_libs_dest}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${vulkan_bin_dir} ${vulkan_bin_dest}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
)
