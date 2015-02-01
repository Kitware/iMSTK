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
# GLM
#

set(GLM_TAG "fad35c4e87d3e72b8911266c9fac5c04f5ee70f7")
set(GLM_REPOSITORY ${git_protocol}://github.com/g-truc/glm.git)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED GLM_DIR AND NOT EXISTS ${GLM_DIR})
  message(FATAL_ERROR "GLM_DIR variable is defined but corresponds to non-existing directory")
endif()

set(GLM_DEPENDENCIES "")

# Include dependent projects if any
SimMedTKCheckDependencies(GLM)

set(proj GLM)

if(NOT DEFINED ${proj}_DIR)

#     message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}-build
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_TAG}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    CMAKE_GENERATOR ${gen}

    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${proj}_DIR ${ep_install_dir})

else()
  SimMedTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

set(SimMedTK_CMAKE_INCLUDE_PATH ${CMAKE_BINARY_DIR}/SuperBuild/${proj}/${sep}${SimMedTK_CMAKE_INCLUDE_PATH})
