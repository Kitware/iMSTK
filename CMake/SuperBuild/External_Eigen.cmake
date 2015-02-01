###########################################################################
#
# Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
#                        Rensselaer Polytechnic Institute
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
###########################################################################
#
#  Copyright (c) Kitware Inc.
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
# Eigen
#

set(Eigen_SHA1 "b826f665bdfe31784795eef79c75540db71ab702")
set(Eigen_URL http://bitbucket.org/eigen/eigen/get/3.2.4.tar.gz)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED Eigen_DIR AND NOT EXISTS ${Eigen_DIR})
  message(FATAL_ERROR "Eigen_DIR variable is defined but corresponds to non-existing directory")
endif()

set(Eigen_DEPENDENCIES "")

# Include dependent projects if any
SimMedTKCheckDependencies(Eigen)

set(proj Eigen)

if(NOT DEFINED ${proj}_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

#     message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    URL ${${proj}_URL}
    URL_HASH SHA1=${${proj}_SHA1}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
      -DBUILD_SHARED_LIBS:BOOL=${SimMedTK_BUILD_SHARED_LIBS}
      -DBUILD_TESTING:BOOL=ON
      -DEIGEN_BUILD_PKGCONFIG:BOOL=OFF
      -DEIGEN_INCLUDE_INSTALL_DIR:PATH=${ep_install_dir}/include
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj})

else()
  SimMedTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

set(SimMedTK_CMAKE_INCLUDE_PATH ${CMAKE_BINARY_DIR}/SuperBuild/${proj}/${sep}${SimMedTK_CMAKE_INCLUDE_PATH})
