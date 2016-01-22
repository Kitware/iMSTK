###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################
#
# VRPN
#

set(VRPN_TAG 807ec7ac6453938af4b0ebeacfce864b37b149ee)
set(VRPN_REPOSITORY https://github.com/vrpn/vrpn.git)

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
if(iMSTK_USE_FALCON)
    set(VRPN_DEPENDENCIES "LibNifalcon;${VRPN_DEPENDENCIES}")
endif()

# Include dependent projects if any
iMSTKCheckDependencies(VRPN)

set(proj VRPN)

if(NOT DEFINED ${proj}_DIR)
  set(VRPN_EP_ARGS)
  if(iMSTK_USE_FALCON)
      list(APPEND VRPN_EP_ARGS
          -DVRPN_USE_LIBNIFALCON:BOOL=ON
          -DLibNifalcon_DIR=${LibNifalcon_DIR}
          -DCMAKE_INCLUDE_PATH=${iMSTK_CMAKE_INCLUDE_PATH}
          )
  endif()

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

  # Set CMake MSVS variable to pass down the external project
  set(CMAKE_MSVC_EXTERNAL_PROJECT_ARGS)
  set(SERVER_ARGS)
  if(USE_VRPN_SERVER)
    if(MSVC)
      list(APPEND CMAKE_MSVC_EXTERNAL_PROJECT_ARGS
          -DLIBUSB1_ROOT_DIR:PATH=${LIBUSB1_ROOT_DIR}
          -DLIBUSB1_INCLUDE_DIR:PATH=${LIBUSB1_INCLUDE_DIR}
          -DLIBUSB1_LIBRARY:PATH=${LIBUSB1_LIBRARY}
      )
    endif()
    list(APPEND SERVER_ARGS
        -DVRPN_BUILD_SERVER_LIBRARY:BOOL=ON
        -DVRPN_BUILD_SERVERS:BOOL=ON
        -DVRPN_USE_HID:BOOL=ON
        )
    if(iMSTK_USE_PHANTOM_OMNI)
        list(APPEND SERVER_ARGS
            -DVRPN_USE_HDAPI:BOOL=ON
            -DVRPN_USE_PHANTOM_SERVER:BOOL=ON)
    endif(iMSTK_USE_PHANTOM_OMNI)
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
      -DCMAKE_INCLUDE_PATH:STRING=${iMSTK_CMAKE_INCLUDE_PATH}
      -DCMAKE_LIBRARY_PATH:STRING=${iMSTK_CMAKE_LIBRARY_PATH}
      -DVRPN_SUBPROJECT_BUILD:BOOL=ON
      -DVRPN_BUILD_CLIENTS:BOOL=OFF
      -DVRPN_BUILD_SERVERS:BOOL=OFF
      -DVRPN_BUILD_CLIENT_LIBRARY:BOOL=ON
      -DVRPN_INSTALL:BOOL=OFF
      -DVRPN_BUILD_PYTHON:BOOL=OFF
      -DVRPN_USE_GPM_MOUSE:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${CMAKE_MSVC_EXTERNAL_PROJECT_ARGS}
      ${OUTPUT_DIRECTORIES}
      ${SERVER_ARGS}
      ${VRPN_EP_ARGS}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    # LOG_DOWNLOAD 1            # Wrap download in script to log output
#     LOG_UPDATE 1              # Wrap update in script to log output
#     LOG_CONFIGURE 1           # Wrap configure in script to log output
#     LOG_BUILD 1               # Wrap build in script to log output
#     LOG_TEST 1                # Wrap test in script to log output
#     LOG_INSTALL 1             # Wrap install in script to log output
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/iMSTK-build/lib)

else()
  iMSTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

set(iMSTK_CMAKE_INCLUDE_PATH ${CMAKE_BINARY_DIR}/SuperBuild/${proj}/${sep}${iMSTK_CMAKE_INCLUDE_PATH})
list(APPEND iMSTK_SUPERBUILD_EP_ARGS -DVRPN_DIR:PATH=${${proj}_DIR})
if(MSVC)
    list(APPEND iMSTK_SUPERBUILD_EP_ARGS
        ${CMAKE_MSVC_EXTERNAL_PROJECT_ARGS}
    )
endif(MSVC)
