include(imstkAddExternalProject)

# Set OpenVR_SOURCE_DIR
imstk_define_external_dirs( OpenVR )

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
set(OpenVR_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

if(WIN32)
  set(openvr_libdir "${OpenVR_SOURCE_DIR}/lib/win64")
  set(openvr_libname "openvr_api.lib")
elseif(APPLE)
  set(openvr_libdir "${OpenVR_SOURCE_DIR}/lib/osx32")
  set(openvr_libname "libopenvr_api.dylib")
elseif(UNIX)
  set(openvr_libdir "${OpenVR_SOURCE_DIR}/lib/linux64")
  set(openvr_libname "libopenvr_api.so")
endif()

if(WIN32)
  set(openvr_dlldir "${OpenVR_SOURCE_DIR}/bin/win64")
  set(openvr_dllname "openvr_api.dll")
elseif(APPLE)
  set(openvr_dlldir "${OpenVR_SOURCE_DIR}/bin/osx64")
  set(openvr_dllname "OpenVR.framework")
elseif(UNIX)
  set(openvr_dlldir "${OpenVR_SOURCE_DIR}/bin/linux64")
  set(openvr_dllname "libopenvr_api.so")
endif()

set(copy_openvr_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${OpenVR_SOURCE_DIR}/headers
  ${OpenVR_INSTALL_DIR}/include/OpenVR
  )
set(copy_openvr_dll_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_dlldir}/${openvr_dllname}
  ${OpenVR_INSTALL_DIR}/bin/${openvr_dllname}
  )
set(copy_openvr_lib_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_libdir}/${openvr_libname}
  ${OpenVR_INSTALL_DIR}/lib/${openvr_libname}
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
imstk_add_external_project( OpenVR
  URL "https://gitlab.kitware.com/iMSTK/openvr/-/archive/v1.12.5/openvr-v1.12.5.tar.gz"
  URL_MD5 "42c4d1c4a194860aaa4e793b009b99c0"
  DOWNLOAD_DIR ${OpenVR_PREFIX}
  SOURCE_DIR ${OpenVR_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${copy_openvr_headers_command}
    COMMAND ${copy_openvr_lib_command}
    COMMAND ${copy_openvr_dll_command}
  RELATIVE_INCLUDE_PATH "headers"
  #VERBOSE
)

