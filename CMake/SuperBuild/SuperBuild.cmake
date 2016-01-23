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

set(iMSTK_DEPENDENCIES VegaFEM Assimp SFML Eigen ThreadPool VTK VRPN)
if(BUILD_TESTING)
  list(APPEND iMSTK_DEPENDENCIES Bandit)
endif()

if(WIN32)
  list(APPEND iMSTK_DEPENDENCIES PTHREAD)
endif(WIN32)

if(iMSTK_USE_OCULUS)
  list(APPEND iMSTK_DEPENDENCIES Oculus)
endif(iMSTK_USE_OCULUS)

#-----------------------------------------------------------------------------
# WARNING - No change should be required after this comment
#           when you are adding a new external project dependency.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Git protocol option
#
option(iMSTK_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)

set(git_protocol "git")
if(NOT iMSTK_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Make sure the binary directory exists
#
if(NOT EXISTS ${iMSTK_BINARY_DIR}/iMSTK-build/bin)
  file(MAKE_DIRECTORY ${iMSTK_BINARY_DIR}/iMSTK-build/bin)
endif()

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#
include(ExternalProject)
include(iMSTKCheckDependencies)

set(ep_install_dir ${CMAKE_BINARY_DIR}/SuperBuild/Install)
set(ep_suffix "-cmake")
set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_suppress_warnings_flags)
if(NOT MSVC)
  set(ep_suppress_warnings_flags "-Wno-old-style-cast -Wno-write-strings")
endif()
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS} ${ep_suppress_warnings_flags}")

set(ep_config_flags)
foreach(type ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${type} type_UP)
  list(APPEND ep_config_flags -DCMAKE_C_FLAGS_${type_UP}:STRING=${CMAKE_C_FLAGS_${type_UP}})
  list(APPEND ep_config_flags -DCMAKE_CXX_FLAGS_${type_UP}:STRING=${CMAKE_CXX_FLAGS_${type_UP}})
endforeach()

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
set(iMSTK_OUTPUT_DIRECTORIES )
set(iMSTK_CMAKE_INCLUDE_PATH)
set(iMSTK_CMAKE_LIBRARY_PATH)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)
    list(APPEND OUTPUT_DIRECTORIES
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND OUTPUT_DIRECTORIES
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND OUTPUT_DIRECTORIES
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND iMSTK_OUTPUT_DIRECTORIES
    -DiMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND iMSTK_OUTPUT_DIRECTORIES
    -DiMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
    list(APPEND iMSTK_OUTPUT_DIRECTORIES
    -DiMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}:STRING=${iMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})

    set(iMSTK_CMAKE_LIBRARY_PATH "${iMSTK_CMAKE_LIBRARY_PATH}${sep}${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}")
    set(iMSTK_CMAKE_LIBRARY_PATH "${iMSTK_CMAKE_LIBRARY_PATH}${sep}${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}")
  endforeach()
else()
  list(APPEND OUTPUT_DIRECTORIES
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  )
  list(APPEND iMSTK_OUTPUT_DIRECTORIES
  -DiMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DiMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DiMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  )
  set(iMSTK_CMAKE_LIBRARY_PATH
  "${iMSTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}${sep}${iMSTK_CMAKE_LIBRARY_PATH}")
  set(iMSTK_CMAKE_LIBRARY_PATH
  "${iMSTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}${sep}${iMSTK_CMAKE_LIBRARY_PATH}")
endif()

#-----------------------------------------------------------------------------
# This variable will contain the list of CMake variable specific to each external project
# that iMSTK depends on.
# The item of this list should have the following form: -D<EP>_DIR:PATH=${<EP>_DIR}
# where '<EP>' is an external project name.
#
set(iMSTK_SUPERBUILD_EP_ARGS
)

#-----------------------------------------------------------------------------
# Check for the dependencies
#
iMSTKCheckDependencies(iMSTK)

#-----------------------------------------------------------------------------
# Set directories where to find the external projects
#
list(APPEND iMSTK_SUPERBUILD_EP_ARGS
  -DCMAKE_INCLUDE_PATH:STRING=${iMSTK_CMAKE_INCLUDE_PATH}
  -DCMAKE_LIBRARY_PATH:STRING=${iMSTK_CMAKE_LIBRARY_PATH}
)
list(REMOVE_DUPLICATES iMSTK_SUPERBUILD_EP_ARGS)

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
# iMSTK Configure
#
SET(proj iMSTK-Configure)

ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  LIST_SEPARATOR ${sep}
  CMAKE_ARGS
    -DiMSTK_SUPERBUILD:BOOL=OFF
#     -DiMSTK_SUPERBUILD_BINARY_DIR:PATH=${iMSTK_BINARY_DIR}
#     -DiMSTK_INSTALL_BIN_DIR:STRING=${iMSTK_INSTALL_BIN_DIR}
#     -DiMSTK_INSTALL_LIB_DIR:STRING=${iMSTK_INSTALL_LIB_DIR}
#     -DiMSTK_INSTALL_INCLUDE_DIR:STRING=${iMSTK_INSTALL_INCLUDE_DIR}
#     -DiMSTK_INSTALL_DOC_DIR:STRING=${iMSTK_INSTALL_DOC_DIR}
    -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
    -DBUILD_SHARED_LIBS:BOOL=${iMSTK_BUILD_SHARED_LIBS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DiMSTK_CXX_FLAGS:STRING=${iMSTK_CXX_FLAGS}
    -DiMSTK_C_FLAGS:STRING=${iMSTK_C_FLAGS}
    # Pass Projects options down to the superbuild #
    # Use Options #
    -DiMSTK_USE_PHANTOM_OMNI:BOOL=${iMSTK_USE_PHANTOM_OMNI}
    -DiMSTK_USE_FALCON:BOOL=${iMSTK_USE_FALCON}
    -DUSE_VRPN_SERVER:BOOL=${USE_VRPN_SERVER}
    -DUSE_VRPN_CLIENT:BOOL=${USE_VRPN_CLIENT}
    -DiMSTK_USE_ADU:BOOL=${iMSTK_USE_ADU}
    -DiMSTK_USE_OCULUS:BOOL=${iMSTK_USE_OCULUS}
    -DiMSTK_USE_NIUSB6008:BOOL=${iMSTK_USE_NIUSB6008}
    # Options #
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DiMSTK_ENABLE_DOCUMENTATION:BOOL=${iMSTK_ENABLE_DOCUMENTATION}
    ${iMSTK_SUPERBUILD_CMAKE_OPTIONS}
    ${iMSTK_OUTPUT_DIRECTORIES}
    ${iMSTK_SUPERBUILD_EP_ARGS}
    #${dependency_args}
  SOURCE_DIR ${iMSTK_SOURCE_DIR}
  BINARY_DIR ${iMSTK_BINARY_DIR}/iMSTK-build
  PREFIX ${iMSTK_BINARY_DIR}/iMSTK${ep_suffix}
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${iMSTK_DEPENDENCIES}
  )

if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
    set(imstk_build_cmd "$(MAKE)")
else()
    set(imstk_build_cmd ${CMAKE_COMMAND} --build ${iMSTK_BINARY_DIR}/iMSTK-build --config ${CMAKE_CFG_INTDIR})
endif()
#-----------------------------------------------------------------------------
# iMSTK
#
#MESSAGE(STATUS SUPERBUILD_EXCLUDE_iMSTKBUILD_TARGET:${SUPERBUILD_EXCLUDE_iMSTKBUILD_TARGET})
if(NOT DEFINED SUPERBUILD_EXCLUDE_iMSTKBUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_iMSTKBUILD_TARGET)
  set(iMSTKBUILD_TARGET_ALL_OPTION "ALL")
else()
  set(iMSTKBUILD_TARGET_ALL_OPTION "")
endif()

# If iMSTK_SUPERBUILD_SUBPROJECT_DASHBOARD is set (by a buildbot slave's
# initial cache, for example), then don't just build the "all" target
if(iMSTK_SUPERBUILD_SUBPROJECT_DASHBOARD)
  set(imstk_target --target Experimental)
else()
  set(imstk_target)
endif()

add_custom_target(iMSTK-build ${iMSTKBUILD_TARGET_ALL_OPTION}
  COMMAND ${imstk_build_cmd} ${imstk_target}
  WORKING_DIRECTORY ${iMSTK_BINARY_DIR}/iMSTK-build
  )
add_dependencies(iMSTK-build iMSTK-Configure)

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of iMSTK project itself
#
add_custom_target(iMSTK
  COMMAND ${imstk_build_cmd}
  WORKING_DIRECTORY ${iMSTK_BINARY_DIR}/iMSTK-build
  )

#-----------------------------------------------------------------------------
# Enable testing in the superbuild and add a dummy test to see if that
# gets the buildbot reporting success.
include(CTest)
enable_testing()

# A dummy test.
# TODO: This *should* test whether we built a valid package.
add_test(
  NAME SuperBuildTest
  COMMAND ${CMAKE_COMMAND} -E echo "Success"
)
