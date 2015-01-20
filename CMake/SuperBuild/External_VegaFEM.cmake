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
# VegaFEM
#

set(proj VegaFEM)
set(${proj}_TAG "6b06c92ee4fa1b208648d8bc8de542329e31cccc")
set(${proj}_REPOSITORY ${git_protocol}://github.com/ricortiz/VegaFEM-cmake.git)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED VegaFEM_DIR AND NOT EXISTS ${VegaFEM_DIR})
  message(FATAL_ERROR "VegaFEM_DIR variable is defined but corresponds to non-existing directory")
endif()

set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
SimMedTKCheckDependencies(${proj})

if(NOT DEFINED VegaFEM_DIR)

  set(VegaFEM_CMAKE_INCLUDE_PATH)
  set(VegaFEM_CMAKE_LIBRARY_PATH)
  set(GL_ARGUMENTS)
  if(GLUT_INCLUDE_DIR AND GLUT_glut_LIBRARY)
    get_filename_component(GLUT_INCLUDE_PREFIX "${GLUT_INCLUDE_DIR}" PATH)
    get_filename_component(GLUT_LIB_PREFIX "${GLUT_glut_LIBRARY}" PATH)
  endif()
  if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
    get_filename_component(GLEW_INCLUDE_PREFIX "${GLEW_INCLUDE_DIR}" PATH)
    get_filename_component(GLEW_LIB_PREFIX "${GLEW_LIBRARY}" PATH)
  endif()

  set(VegaFEM_CMAKE_INCLUDE_PATH ${GLUT_INCLUDE_PREFIX} ${GLEW_INCLUDE_PREFIX})
  set(VegaFEM_CMAKE_LIBRARY_PATH ${GLUT_LIB_PREFIX} ${GLEW_LIB_PREFIX})

  if(VegaFEM_CMAKE_INCLUDE_PATH AND VegaFEM_CMAKE_LIBRARY_PATH)
    set(GL_ARGUMENTS -DCMAKE_INCLUDE_PATH=${VegaFEM_CMAKE_INCLUDE_PATH})
    set(GL_ARGUMENTS ${GL_ARGUMENTS} -DCMAKE_LIBRARY_PATH=${VegaFEM_CMAKE_LIBRARY_PATH})
  endif()
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
    BINARY_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}-build
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_TAG}
    UPDATE_COMMAND ""
#     INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
      -DBUILD_SHARED_LIBS:BOOL=${SimMedTK_BUILD_SHARED_LIBS}
      -DVegaFEM_ENABLE_PTHREADS_SUPPORT:BOOL=ON
      -DVegaFEM_ENABLE_OpenGL_SUPPORT:BOOL=ON
      -DVegaFEM_BUILD_MODEL_REDUCTION:BOOL=OFF
      -DVegaFEM_BUILD_UTILITIES:BOOL=OFF
#       -DGLEW_INCLUDE_DIR:PATH="${GLEW_INCLUDE_DIR}"
#       -DGLEW_LIBRARY:STRING="${GLEW_LIBRARY}"
#       -DGLUT_INCLUDE_DIR:PATH="${GLUT_INCLUDE_DIR}"
#       -DGLUT_LIBRARIES:STRING="${GLUT_LIBRARIES}"
      ${GL_ARGUMENTS}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    DEPENDS
      ${VegaFEM_DEPENDENCIES}
    )
  set(${proj}_DIR ${ep_install_dir}/lib/cmake/VegaFEM)

else()
  SimMedTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

list(APPEND SimMedTK_SUPERBUILD_EP_ARGS -D${proj}_DIR:PATH=${${proj}_DIR})
