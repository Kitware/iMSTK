if(NOT WIN32)
  message(WARNING "System not supported. For nownly Windows is supported for External_OPENVR.cmake.")
  return()
endif()

set(_version "1.0.9")

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

set(copy_openvr_dll_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_dlldir}/${openvr_dllname}
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${openvr_dllname}
  )
set(copy_openvr_lib_command
  ${CMAKE_COMMAND} -E copy
  ${openvr_libdir}/${openvr_libname}
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${openvr_libname}
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( openvr
  URL "https://github.com/ValveSoftware/openvr/archive/v${_version}.tar.gz"
  URL_MD5 "0de39e805155cf4e0e907bb181924c48"
  DOWNLOAD_DIR ${OPENVR_PREFIX}
  SOURCE_DIR ${OPENVR_EXTRACT_DIR}
  BINARY_DIR ${OPENVR_EXTRACT_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    ${copy_openvr_lib_command}
    COMMAND ${copy_openvr_dll_command}
  RELATIVE_INCLUDE_PATH "headers"
  #VERBOSE
  )
