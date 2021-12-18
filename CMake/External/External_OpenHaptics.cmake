if(NOT WIN32)
  message(WARNING "System not supported. Currently, only Windows is supported for External_OpenHaptics.cmake.")
  return()
endif()

if(NOT DEFINED OPENHAPTICS_ROOT_DIR OR NOT EXISTS ${OPENHAPTICS_ROOT_DIR})
  set(OPENHAPTICS_ROOT_DIR "$ENV{OH_SDK_BASE}" CACHE PATH "Path to OpenHaptics install directory." FORCE)
endif()
if(NOT EXISTS ${OPENHAPTICS_ROOT_DIR})
  message(FATAL_ERROR "\nCan not support Phantom Omni without OpenHaptics.\nSet OPENHAPTICS_ROOT_DIR to OpenHaptics installation directory.\n\n")
endif()

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(OpenHaptics_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

if(WIN32)
  set(open_haptics_libdir "Win32")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(open_haptics_libdir "x64")
  endif()
  set(lib_ext "lib")
  set(bin_ext "dll")
endif()

# Header Directories
set(open_haptics_headers_dest ${OpenHaptics_INSTALL_DIR}/include/OpenHaptics)
set(open_haptics_headers_dir ${OPENHAPTICS_ROOT_DIR}/include/)
set(open_haptics_util_headers_dir ${OPENHAPTICS_ROOT_DIR}/utilities/include/)

# Library Directories
set(open_haptics_libs_dest ${OpenHaptics_INSTALL_DIR}/lib/)
set(open_haptics_libs_dir ${OPENHAPTICS_ROOT_DIR}/lib/${open_haptics_libdir}/Release)
set(open_haptics_util_libs_dir ${OPENHAPTICS_ROOT_DIR}/utilities/lib/${open_haptics_libdir}/Release)

# Binary Directory
set(open_haptics_bin_dest ${OpenHaptics_INSTALL_DIR}/bin/)

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

# Workaround issue in "imstk_add_external_project" by explicitly set the
# OpenHaptics_SOURCE_DIR variable.
# This ensures the call to "imstk_define_external_dirs" done in "imstk_add_external_project"
# is not setting a default value because OpenHaptics_DIR is not defined.
set(OpenHaptics_SOURCE_DIR ${OPENHAPTICS_ROOT_DIR})

include(imstkAddExternalProject)
imstk_add_external_project( OpenHaptics
  SOURCE_DIR ${OpenHaptics_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${open_haptics_headers_dir} ${open_haptics_headers_dest}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${open_haptics_util_headers_dir} ${open_haptics_headers_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_libs_dir}/hd.${lib_ext} ${open_haptics_libs_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_libs_dir}/hd.${bin_ext} ${open_haptics_bin_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_libs_dir}/hl.${lib_ext} ${open_haptics_libs_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_libs_dir}/hl.${bin_ext} ${open_haptics_bin_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_util_libs_dir}/hdu.${lib_ext} ${open_haptics_libs_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${open_haptics_util_libs_dir}/hlu.${lib_ext} ${open_haptics_libs_dest}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
)
