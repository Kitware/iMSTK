if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_Haply.cmake.")
  return()
endif()

# Attempt to initialize HAPLY_ROOT_DIR based on Haply_SDK_BASE env. variable
if(NOT DEFINED HAPLY_ROOT_DIR)

  # Check if Haply_SDK_BASE is defined
  set(env_var_defined FALSE)
  set(msg "Checking if Haply_SDK_BASE env. variable is defined")
  message(STATUS "${msg}")
  if(DEFINED ENV{Haply_SDK_BASE})
    set(env_var_defined TRUE)
  endif()
  message(STATUS "${msg} - ${env_var_defined}")

  # Check if path associated with Haply_SDK_BASE exists
  if(env_var_defined)
    set(env_var_path_exist FALSE)
    set(msg "Checking if Haply_SDK_BASE env. variable is set to existing path")
    message(STATUS "${msg}")
    if(EXISTS "$ENV{Haply_SDK_BASE}")
      set(env_var_path_exist TRUE)
    endif()
    message(STATUS "${msg} - ${env_var_path_exist}")

    if(env_var_path_exist)
      set(HAPLY_ROOT_DIR "$ENV{Haply_SDK_BASE}" CACHE PATH "Path to Haply install directory.")
      message(STATUS "Setting HAPLY_ROOT_DIR to ${HAPLY_ROOT_DIR}")
    endif()
  endif()

  unset(env_var_defined)
  unset(env_var_path_exist)
endif()

# Sanity checks
if(DEFINED HAPLY_ROOT_DIR AND NOT EXISTS ${HAPLY_ROOT_DIR})
  message(FATAL_ERROR "HAPLY_ROOT_DIR variable is defined but corresponds to nonexistent directory")
endif()

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(Haply_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

if (WIN32 AND MSVC_VERSION LESS_EQUAL 1930)
  message(FATAL_ERROR "iMSTK only works with Haply HardwareAPI 0.0.5, this version of Haply only works with MSVC version 1930 (vs2022) or greater")
endif()

if(WIN32)
  set(lib_ext "lib")
  set(bin_ext "dll")
endif()

# Header Directories
set(haply_headers_dest ${Haply_INSTALL_DIR}/include/Haply)
set(haply_headers_dir ${HAPLY_ROOT_DIR}/)

# Library Directories
set(haply_libs_dest ${Haply_INSTALL_DIR}/lib)
set(haply_libs_dir ${HAPLY_ROOT_DIR}/)

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------

# Workaround issue in "imstk_add_external_project" by explicitly set the
# Haply_SOURCE_DIR variable.
# This ensures the call to "imstk_define_external_dirs" done in "imstk_add_external_project"
# is not setting a default value because Haply_SOURCE_DIR is not defined.
set(Haply_SOURCE_DIR ${HAPLY_ROOT_DIR})

include(imstkAddExternalProject)
imstk_add_external_project( Haply
  SOURCE_DIR ${Haply_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${haply_headers_dir} ${haply_headers_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${haply_libs_dir}/Haply.HardwareAPI.${lib_ext} ${haply_libs_dest}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
)
