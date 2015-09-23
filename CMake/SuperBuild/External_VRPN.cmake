###########################################################################
# Copyright 2015 by Kitware and RPI. See toplevel LICENSE.txt for details.
###########################################################################

set(VRPN_TAG f733f16b7f978ffdcc722a453a0ed55e8c634cd7)
set(VRPN_REPOSITORY https://github.com/sradigan/vrpn.git)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED VRPN_DIR AND NOT EXISTS ${VRPN_DIR})
  message(FATAL_ERROR "VRPN_DIR variable is defined but corresponds to non-existing directory")
endif()

set(VRPN_DEPENDENCIES "")

# Include dependent projects if any
SimMedTKCheckDependencies(VRPN)

set(proj VRPN)

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

  set(SERVER_ARGS)
  if(USE_VRPN_SERVER)
    list(APPEND SERVER_ARGS
#         -DVRPN_BUILD_SERVERS:BOOL=ON
        -DVRPN_BUILD_SERVER_LIBRARY:BOOL=ON
        -DVRPN_GPL_SERVER:BOOL=ON
        -DVRPN_USE_HID:BOOL=ON
        -DVRPN_USE_LIBUSB_1_0:BOOL=ON)
    if(SimMedTK_USE_PHANTOM_OMNI)
        list(APPEND SERVER_ARGS
            -DVRPN_USE_HDAPI:BOOL=ON
            -DVRPN_USE_PHANTOM_SERVER:BOOL=ON)
    endif(SimMedTK_USE_PHANTOM_OMNI)
  endif()

#   message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}-build
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_TAG}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DVRPN_USE_SHARED_LIBRARY:BOOL=OFF
      -DCMAKE_INCLUDE_PATH:STRING=${SimMedTK_CMAKE_INCLUDE_PATH}
      -DCMAKE_LIBRARY_PATH:STRING=${SimMedTK_CMAKE_LIBRARY_PATH}
      -DVRPN_SUBPROJECT_BUILD:BOOL=ON
      -DVRPN_BUILD_CLIENT_LIBRARY:BOOL=ON
      -DVRPN_BUILD_SERVER_LIBRARY:BOOL=ON
      -DVRPN_INSTALL:BOOL=OFF
      -DVRPN_BUILD_PYTHON:BOOL=OFF
      -DVRPN_USE_GPM_MOUSE:BOOL=OFF
      -DVRPN_USE_LIBNIFALCON:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${OUTPUT_DIRECTORIES}
      ${SERVER_ARGS}
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
list(APPEND SimMedTK_SUPERBUILD_EP_ARGS -DVRPN_DIR:PATH=${${proj}_DIR})
