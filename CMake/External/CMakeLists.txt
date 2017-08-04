set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH} )
include(ExternalProject)
include(imstkSolveDependencies)

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------
if(NOT GIT_EXECUTABLE)
  find_package(Git REQUIRED)
endif()

option(${PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)
set(git_protocol "git")
if(NOT ${PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Output Directories
#-----------------------------------------------------------------------------
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${${PROJECT_NAME}_LIB_DIR})
endif()
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${${PROJECT_NAME}_LIB_DIR})
endif()
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${${PROJECT_NAME}_BIN_DIR})
endif()

mark_as_superbuild(
  VARS
    CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH
    CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH
  ALL_PROJECTS
  )

#-----------------------------------------------------------------------------
# Search Directories
#-----------------------------------------------------------------------------
list(APPEND CMAKE_LIBRARY_PATH
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  )
list(APPEND CMAKE_PROGRAM_PATH
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  )

list(APPEND CMAKE_INCLUDE_PATH "") # Populated in each External_*.cmake

mark_as_superbuild(
  VARS
    CMAKE_LIBRARY_PATH
    CMAKE_PROGRAM_PATH
    CMAKE_INCLUDE_PATH
  ALL_PROJECTS
  )

#-----------------------------------------------------------------------------
# CMake global args
#-----------------------------------------------------------------------------
mark_as_superbuild(
  VARS
    CMAKE_BUILD_TYPE
    CMAKE_C_COMPILER
    CMAKE_CXX_COMPILER
    CMAKE_C_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_EXE_LINKER_FLAGS
    CMAKE_SHARED_LINKER_FLAGS
    CMAKE_CXX_STANDARD
    CMAKE_CXX_STANDARD_REQUIRED
    THREADS_PREFER_PTHREAD_FLAG
  ALL_PROJECTS
  )

#-----------------------------------------------------------------------------
# CMake args if Apple
#-----------------------------------------------------------------------------
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  mark_as_superbuild(
    VARS
      CMAKE_OSX_ARCHITECTURES
      CMAKE_OSX_SYSROOT
      CMAKE_OSX_DEPLOYMENT_TARGET
      CMAKE_MACOSX_RPATH
    ALL_PROJECTS
    )
endif()

#-----------------------------------------------------------------------------
# Keep track of include path for superbuild
#-----------------------------------------------------------------------------
set(CMAKE_INCLUDE_PATH )
mark_as_superbuild(VARS CMAKE_INCLUDE_PATH ALL_PROJECTS)

#-----------------------------------------------------------------------------
# Solve dependencies
#-----------------------------------------------------------------------------
set(EXTERNAL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Location of the "External_*" files

ExternalProject_Include_Dependencies( ${PROJECT_NAME}
  DEPENDS_VAR ${PROJECT_NAME}_DEPENDENCIES
  EP_ARGS_VAR ${PROJECT_NAME}_EP_ARGS
  SUPERBUILD_VAR ${PROJECT_NAME}_SUPERBUILD
  )

#-----------------------------------------------------------------------------
# Inner build of the main project
#-----------------------------------------------------------------------------
ExternalProject_Add( ${PROJECT_NAME}
  ${${PROJECT_NAME}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/Innerbuild
  CMAKE_ARGS
    -D${PROJECT_NAME}_SUPERBUILD:BOOL=OFF
    -D${PROJECT_NAME}_USE_OMNI:BOOL=${${PROJECT_NAME}_USE_OMNI}
    -D${PROJECT_NAME}_USE_ODE:BOOL=${${PROJECT_NAME}_USE_ODE}
    -DODE_ROOT_DIR:PATH=${ODE_ROOT_DIR}
    -D${PROJECT_NAME}_USE_Vulkan:BOOL=${${PROJECT_NAME}_USE_Vulkan}
    -D${PROJECT_NAME}_ENABLE_AUDIO:BOOL=${${PROJECT_NAME}_ENABLE_AUDIO}
  DEPENDS ${${PROJECT_NAME}_DEPENDENCIES}
  )
