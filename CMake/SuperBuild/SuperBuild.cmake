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

set(SimMedTK_DEPENDENCIES VegaFEM Assimp GLFW Eigen GLEW GLUT GLM SFML)
if(WIN32)
  list(APPEND SimMedTK_DEPENDENCIES PTHREAD)
endif(WIN32)

#-----------------------------------------------------------------------------
# WARNING - No change should be required after this comment
#           when you are adding a new external project dependency.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Git protocol option
#
option(SimMedTK_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)

set(git_protocol "git")
if(NOT SimMedTK_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Make sure the binary directory exists
#
if(NOT EXISTS ${SimMedTK_BINARY_DIR}/SimMedTK-build/bin)
  file(MAKE_DIRECTORY ${SimMedTK_BINARY_DIR}/SimMedTK-build/bin)
endif()

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#
include(ExternalProject)
include(SimMedTKCheckDependencies)

set(ep_install_dir ${CMAKE_BINARY_DIR}/SuperBuild/Install)
set(ep_suffix "-cmake")
set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")

#----------------------------------------------------------------------------
# Compute -G arg for configuring external projects with the same CMake generator:
#
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#----------------------------------------------------------------------------
# Use to pass list to the ExternalProject_Add CMAKE_ARGS directive:
#
set(sep "^^")

#----------------------------------------------------------------------------
# Set output directories for external projects
#
set(OUTPUT_DIRECTORIES )
set(SimMedTK_OUTPUT_DIRECTORIES )
set(SimMedTK_CMAKE_INCLUDE_PATH)
set(SimMedTK_CMAKE_LIBRARY_PATH)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)
    list(APPEND OUTPUT_DIRECTORIES -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND OUTPUT_DIRECTORIES -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND OUTPUT_DIRECTORIES -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND SimMedTK_OUTPUT_DIRECTORIES -DSimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND SimMedTK_OUTPUT_DIRECTORIES -DSimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND SimMedTK_OUTPUT_DIRECTORIES -DSimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${SimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})

    set(SimMedTK_CMAKE_LIBRARY_PATH "${SimMedTK_CMAKE_LIBRARY_PATH}${sep}${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}")
    set(SimMedTK_CMAKE_LIBRARY_PATH "${SimMedTK_CMAKE_LIBRARY_PATH}${sep}${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}")
  endforeach()
else()
  list(APPEND OUTPUT_DIRECTORIES
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  )
  list(APPEND SimMedTK_OUTPUT_DIRECTORIES
  -DSimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DSimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DSimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  )
  set(SimMedTK_CMAKE_LIBRARY_PATH "${SimMedTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}${sep}${SimMedTK_CMAKE_LIBRARY_PATH}")
  set(SimMedTK_CMAKE_LIBRARY_PATH "${SimMedTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}${sep}${SimMedTK_CMAKE_LIBRARY_PATH}")
endif()

#-----------------------------------------------------------------------------
# This variable will contain the list of CMake variable specific to each external project
# that SimMedTK depends on.
# The item of this list should have the following form: -D<EP>_DIR:PATH=${<EP>_DIR}
# where '<EP>' is an external project name.
#
set(SimMedTK_SUPERBUILD_EP_ARGS
  -DCMAKE_LIBRARY_PATH:STRING=${SimMedTK_CMAKE_LIBRARY_PATH})

#-----------------------------------------------------------------------------
# Check for the dependencies
#
SimMedTKCheckDependencies(SimMedTK)

#-----------------------------------------------------------------------------
# Set directories where to find the external projects
#
list(APPEND SimMedTK_SUPERBUILD_EP_ARGS
  -DCMAKE_INCLUDE_PATH:STRING=${SimMedTK_CMAKE_INCLUDE_PATH}
)

#-----------------------------------------------------------------------------
# Set CMake OSX variable to pass down the external project
#
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

#-----------------------------------------------------------------------------
# Set CMake Windows variable to pass down the external project
#
set(CMAKE_MSVC_EXTERNAL_PROJECT_ARGS)
if(WIN32)
  list(APPEND CMAKE_MSVC_EXTERNAL_PROJECT_ARGS
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE})
endif(WIN32)

#-----------------------------------------------------------------------------
# SimMedTK Configure
#
SET(proj SimMedTK-Configure)

ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  LIST_SEPARATOR ${sep}
  CMAKE_ARGS
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    ${CMAKE_MSVC_EXTERNAL_PROJECT_ARGS}
    -DSimMedTK_SUPERBUILD:BOOL=OFF
    -DSimMedTK_SUPERBUILD_BINARY_DIR:PATH=${SimMedTK_BINARY_DIR}
#     -DSimMedTK_INSTALL_BIN_DIR:STRING=${SimMedTK_INSTALL_BIN_DIR}
#     -DSimMedTK_INSTALL_LIB_DIR:STRING=${SimMedTK_INSTALL_LIB_DIR}
#     -DSimMedTK_INSTALL_INCLUDE_DIR:STRING=${SimMedTK_INSTALL_INCLUDE_DIR}
#     -DSimMedTK_INSTALL_DOC_DIR:STRING=${SimMedTK_INSTALL_DOC_DIR}
    -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
    -DSimMedTK_BUILD_SHARED_LIBS:BOOL=${SimMedTK_BUILD_SHARED_LIBS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DSimMedTK_CXX_FLAGS:STRING=${SimMedTK_CXX_FLAGS}
    -DSimMedTK_C_FLAGS:STRING=${SimMedTK_C_FLAGS}
    -DSimMedTK_USE_PHANTOM_OMNI:BOOL=${SimMedTK_USE_PHANTOM_OMNI}
    -DSimMedTK_USE_ADU:BOOL=${SimMedTK_USE_ADU}
    -DSimMedTK_USE_NIUSB6008:BOOL=${SimMedTK_USE_NIUSB6008}
    ${SimMedTK_OUTPUT_DIRECTORIES}
    ${SimMedTK_SUPERBUILD_EP_ARGS}
    #${dependency_args}
  SOURCE_DIR ${SimMedTK_SOURCE_DIR}
  BINARY_DIR ${SimMedTK_BINARY_DIR}/SimMedTK-build
  PREFIX ${SimMedTK_BINARY_DIR}/SimMedTK${ep_suffix}
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${SimMedTK_DEPENDENCIES}
  )

if(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(simmedtk_build_cmd "$(MAKE)")
else()
  set(simmedtk_build_cmd ${CMAKE_COMMAND} --build ${SimMedTK_BINARY_DIR}/SimMedTK-build --config ${CMAKE_CFG_INTDIR})
endif()

#-----------------------------------------------------------------------------
# SimMedTK
#
#MESSAGE(STATUS SUPERBUILD_EXCLUDE_SimMedTKBUILD_TARGET:${SUPERBUILD_EXCLUDE_SimMedTKBUILD_TARGET})
if(NOT DEFINED SUPERBUILD_EXCLUDE_SimMedTKBUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_SimMedTKBUILD_TARGET)
  set(SimMedTKBUILD_TARGET_ALL_OPTION "ALL")
else()
  set(SimMedTKBUILD_TARGET_ALL_OPTION "")
endif()

add_custom_target(SimMedTK-build ${SimMedTKBUILD_TARGET_ALL_OPTION}
  COMMAND ${simmedtk_build_cmd}
  WORKING_DIRECTORY ${SimMedTK_BINARY_DIR}/SimMedTK-build
  )
add_dependencies(SimMedTK-build SimMedTK-Configure)

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of SimMedTK project itself
#
add_custom_target(SimMedTK
  COMMAND ${simmedtk_build_cmd}
  WORKING_DIRECTORY ${SimMedTK_BINARY_DIR}/SimMedTK-build
  )
