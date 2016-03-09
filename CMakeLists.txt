###########################################################################
#
#  Library: iMSTK
#
#  Copyright (c) Kitware, Inc.
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

cmake_minimum_required(VERSION 3.5)

project(iMSTK VERSION 0.0.1 LANGUAGES CXX)

#-----------------------------------------------------------------------------
# Update CMake module path & cmake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_MODULE_PATH}
    )
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Project build directories
#-----------------------------------------------------------------------------
set(${PROJECT_NAME}_BIN_DIR "bin")
set(${PROJECT_NAME}_LIB_DIR "lib/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
set(${PROJECT_NAME}_INCLUDE_DIR "include/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
set(${PROJECT_NAME}_SHARE_DIR "share/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")

#-----------------------------------------------------------------------------
# Project install directories
#-----------------------------------------------------------------------------
if(APPLE)
  set(${PROJECT_NAME}_INSTALL_ROOT "${${PROJECT_NAME}_MAIN_PROJECT_APPLICATION_NAME}.app/Contents/") # Set to create Bundle
else()
  set(${PROJECT_NAME}_INSTALL_ROOT "./")
endif()
set(${PROJECT_NAME}_INSTALL_BIN_DIR "${${PROJECT_NAME}_INSTALL_ROOT}${${PROJECT_NAME}_BIN_DIR}")
set(${PROJECT_NAME}_INSTALL_LIB_DIR "${${PROJECT_NAME}_INSTALL_ROOT}${${PROJECT_NAME}_LIB_DIR}")
set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}${${PROJECT_NAME}_INCLUDE_DIR}")
set(${PROJECT_NAME}_INSTALL_SHARE_DIR "${${PROJECT_NAME}_INSTALL_ROOT}${${PROJECT_NAME}_SHARE_DIR}")

#-----------------------------------------------------------------------------
# C++11 Support
#-----------------------------------------------------------------------------
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#--------------------------------------------------------------------------
# Add Source code subdirectory
#--------------------------------------------------------------------------
add_subdirectory(Base/Core)
add_subdirectory(Base/Scene)
add_subdirectory(Base/SimulationManager)

#--------------------------------------------------------------------------
# Export Targets
#--------------------------------------------------------------------------
export(EXPORT iMSTK_TARGETS
  FILE ${CMAKE_CURRENT_BINARY_DIR}/iMSTKTargets.cmake
  NAMESPACE imstk::
  )

install(EXPORT iMSTK_TARGETS
  FILE iMSTKTargets.cmake
  NAMESPACE imstk::
  COMPONENT Development
  DESTINATION ${${PROJECT_NAME}_INSTALL_SHARE_DIR}
  )
