include(imstkAddExternalProject)

# Set OpenVR_SOURCE_DIR and OpenVR_PREFIX
imstk_define_external_dirs( OpenVR )

# Directories and filenames specific to the OpenVR archive layout
set(OpenVR_INC_DIR "headers")
if(WIN32)
  set(OpenVR_DLL_DIR "bin/win64")
  set(OpenVR_DLL_NAME "openvr_api.dll")
  set(OpenVR_LIB_DIR "lib/win64")
  set(OpenVR_LIB_NAME "openvr_api.lib")
elseif(APPLE)
  set(OpenVR_DLL_DIR "bin/osx64")
  set(OpenVR_DLL_NAME "OpenVR.framework")
  set(OpenVR_LIB_DIR "lib/osx32")
  set(OpenVR_LIB_NAME "libopenvr_api.dylib")
elseif(UNIX)
  set(OpenVR_DLL_DIR "bin/linux64")
  set(OpenVR_DLL_NAME "libopenvr_api.so")
  set(OpenVR_LIB_DIR "lib/linux64")
  set(OpenVR_LIB_NAME "libopenvr_api.so")
endif()

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(OpenVR_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

  set(copy_openvr_headers_command
    ${CMAKE_COMMAND} -E copy_directory
      ${OpenVR_SOURCE_DIR}/${OpenVR_INC_DIR}
      ${OpenVR_INSTALL_DIR}/include/OpenVR
    )
  set(copy_openvr_dll_command
    ${CMAKE_COMMAND} -E copy
      ${OpenVR_SOURCE_DIR}/${OpenVR_DLL_DIR}/${OpenVR_DLL_NAME}
      ${OpenVR_INSTALL_DIR}/bin/${OpenVR_DLL_NAME}
    )
  set(copy_openvr_lib_command
    ${CMAKE_COMMAND} -E copy
      ${OpenVR_SOURCE_DIR}/${OpenVR_LIB_DIR}/${OpenVR_LIB_NAME}
      ${OpenVR_INSTALL_DIR}/lib/${OpenVR_LIB_NAME}
    )

  set(OpenVR_INSTALL_COMMAND
    INSTALL_COMMAND
      COMMAND ${copy_openvr_headers_command}
      COMMAND ${copy_openvr_lib_command}
      COMMAND ${copy_openvr_dll_command}
    )
else()
  set(OpenVR_INSTALL_COMMAND
    INSTALL_COMMAND
      COMMAND ${SKIP_STEP_COMMAND}
    )
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

# Download options
if(NOT DEFINED iMSTK_OpenVR_GIT_SHA)
  set(iMSTK_OpenVR_GIT_SHA "v1.12.5")
endif()
if(NOT DEFINED iMSTK_OpenVR_GIT_REPOSITORY)
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    URL https://gitlab.kitware.com/iMSTK/openvr/-/archive/${iMSTK_OpenVR_GIT_SHA}/openvr-${iMSTK_OpenVR_GIT_SHA}.tar.gz
    URL_HASH MD5=42c4d1c4a194860aaa4e793b009b99c0
    )
else()
  set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
    GIT_REPOSITORY ${iMSTK_OpenVR_GIT_REPOSITORY}
    GIT_TAG ${iMSTK_OpenVR_GIT_SHA}
    )
endif()

imstk_add_external_project( OpenVR
  ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
  DOWNLOAD_DIR ${OpenVR_PREFIX}
  SOURCE_DIR ${OpenVR_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  ${OpenVR_INSTALL_COMMAND}
  RELATIVE_INCLUDE_PATH "${OpenVR_INC_DIR}"
  #VERBOSE
)

