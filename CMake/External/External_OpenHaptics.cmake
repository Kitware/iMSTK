if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_OpenHaptics.cmake.")
  return()
endif()

# Attempt to initialize OPENHAPTICS_ROOT_DIR based on OH_SDK_BASE env. variable
if(NOT DEFINED OPENHAPTICS_ROOT_DIR)

  # Check if OH_SDK_BASE is defined
  set(env_var_defined FALSE)
  set(msg "Checking if OH_SDK_BASE env. variable is defined")
  message(STATUS "${msg}")
  if(DEFINED ENV{OH_SDK_BASE})
    set(env_var_defined TRUE)
  endif()
  message(STATUS "${msg} - ${env_var_defined}")

  # Check if path associated with OH_SDK_BASE exists
  if(env_var_defined)
    set(env_var_path_exist FALSE)
    set(msg "Checking if OH_SDK_BASE env. variable is set to existing path")
    message(STATUS "${msg}")
    if(EXISTS "$ENV{OH_SDK_BASE}")
      set(env_var_path_exist TRUE)
    endif()
    message(STATUS "${msg} - ${env_var_path_exist}")

    if(env_var_path_exist)
      set(OPENHAPTICS_ROOT_DIR "$ENV{OH_SDK_BASE}" CACHE PATH "Path to OpenHaptics install directory.")
      message(STATUS "Setting OPENHAPTICS_ROOT_DIR to ${OPENHAPTICS_ROOT_DIR}")
    endif()
  endif()

  unset(env_var_defined)
  unset(env_var_path_exist)
endif()

# Sanity checks
if(DEFINED OPENHAPTICS_ROOT_DIR AND NOT EXISTS ${OPENHAPTICS_ROOT_DIR})
  message(FATAL_ERROR "OPENHAPTICS_ROOT_DIR variable is defined but corresponds to nonexistent directory")
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
