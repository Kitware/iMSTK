###########################################################################
# Copyright 2015 by Kitware and RPI. See toplevel LICENSE.txt for details.
###########################################################################

set(LibNifalcon_TAG 7e98c9f2bdf936d236260176921c865f8fd1b108)
set(LibNifalcon_REPOSITORY git@github.com:qdot/libnifalcon.git)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED LibNifalcon_DIR AND NOT EXISTS ${LibNifalcon_DIR})
  message(FATAL_ERROR "LibNifalcon_DIR variable is defined but corresponds to non-existing directory")
endif()

set(LibNifalcon_DEPENDENCIES "")

# Include dependent projects if any
SimMedTKCheckDependencies(LibNifalcon)

set(proj LibNifalcon)

if(NOT DEFINED ${proj}_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT:PATH=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
      -DCMAKE_MACOSX_RPATH:BOOL=ON
    )
  endif()

#   message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}-build
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_TAG}
    GIT_SUBMODULES cmake_modules
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED:BOOL=${SimMedTK_BUILD_SHARED_LIBS}
      -DCMAKE_INCLUDE_PATH:STRING=${SimMedTK_CMAKE_INCLUDE_PATH}
      -DCMAKE_LIBRARY_PATH:STRING=${SimMedTK_CMAKE_LIBRARY_PATH}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_SWIG_BINDINGS:BOOL=OFF
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${OUTPUT_DIRECTORIES}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    LOG_DOWNLOAD 1            # Wrap download in script to log output
#     LOG_UPDATE 1              # Wrap update in script to log output
#     LOG_CONFIGURE 1           # Wrap configure in script to log output
#     LOG_BUILD 1               # Wrap build in script to log output
#     LOG_TEST 1                # Wrap test in script to log output
#     LOG_INSTALL 1             # Wrap install in script to log output
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/SimMedTK-build/lib)

else()
  SimMedTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

set(SimMedTK_CMAKE_INCLUDE_PATH ${CMAKE_BINARY_DIR}/SuperBuild/${proj}/${sep}${SimMedTK_CMAKE_INCLUDE_PATH})
list(APPEND SimMedTK_SUPERBUILD_EP_ARGS -DLibNifalcon_DIR:PATH=${${proj}_DIR})
