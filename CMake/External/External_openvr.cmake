set(_version "1.12.5")
#-----------------------------------------------------------------------------
# Set project prefix path
#-----------------------------------------------------------------------------
set(OPENVR_PREFIX ${CMAKE_BINARY_DIR}/External/openvr)
set(OPENVR_EXTRACT_DIR ${OPENVR_PREFIX}/v${_version})

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(WIN32)
  set(openvr_libdir "${OPENVR_EXTRACT_DIR}/lib/win64")
  set(openvr_libname "openvr_api.lib")
elseif(APPLE)
  set(openvr_libdir "${OPENVR_EXTRACT_DIR}/lib/osx32")
  set(openvr_libname "libopenvr_api.dylib")
elseif(UNIX)
  set(openvr_libdir "${OPENVR_EXTRACT_DIR}/lib/linux64")
  set(openvr_libname "libopenvr_api.so")
endif()

if(WIN32)
  set(openvr_dlldir "${OPENVR_EXTRACT_DIR}/bin/win64")
  set(openvr_dllname "openvr_api.dll")
elseif(APPLE)
  set(openvr_dlldir "${OPENVR_EXTRACT_DIR}/bin/osx64")
  set(openvr_dllname "OpenVR.framework")
elseif(UNIX)
  set(openvr_dlldir "${OPENVR_EXTRACT_DIR}/bin/linux64")
  set(openvr_dllname "libopenvr_api.so")
endif()

set(copy_openvr_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${OPENVR_EXTRACT_DIR}/headers
  ${CMAKE_INSTALL_PREFIX}/include/openvr
  )
set(copy_openvr_dll_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_dlldir}/${openvr_dllname}
  ${CMAKE_INSTALL_PREFIX}/bin/${openvr_dllname}
  )
set(copy_openvr_lib_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_libdir}/${openvr_libname}
  ${CMAKE_INSTALL_PREFIX}/lib/${openvr_libname}
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( openvr
  URL "https://github.com/ValveSoftware/openvr/archive/v${_version}.tar.gz"
  URL_MD5 "fabbc1ad83d7c382080ca00b878e6209"
  DOWNLOAD_DIR ${OPENVR_PREFIX}
  SOURCE_DIR ${OPENVR_EXTRACT_DIR}
  BINARY_DIR ${OPENVR_EXTRACT_DIR}
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

