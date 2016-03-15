cmake_minimum_required(VERSION 3.5)

project(iMSTK VERSION 0.0.1 LANGUAGES CXX)

#-----------------------------------------------------------------------------
# Update CMake module path & cmake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Superbuild
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#-----------------------------------------------------------------------------
# Define ThirdParty dependencies
#-----------------------------------------------------------------------------
# KWStyle
option(${PROJECT_NAME}_USE_KWSTYLE "Use KWStyle to check for coding standards violations." ON)
option(USE_SYSTEM_KWSTYLE "Exclude KWStyle from superbuild and use an existing build." OFF)
mark_as_advanced(USE_SYSTEM_KWSTYLE)

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------
if(NOT GIT_EXECUTABL )
  find_package(Git REQUIRED)
endif()

option(${PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)
set(git_protocol "git")
if(NOT ${PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Superbuild
#-----------------------------------------------------------------------------
option(${PROJECT_NAME}_SUPERBUILD "Build ${PROJECT_NAME} and the projects it depends on." ON)
mark_as_advanced(${PROJECT_NAME}_SUPERBUILD)
set(${PROJECT_NAME}_BINARY_INNER_SUBDIR ${PROJECT_NAME}-build)

if(${PROJECT_NAME}_SUPERBUILD)
  include(External_KWStyle)
endif()

#-----------------------------------------------------------------------------
# Include ThirdParty dependencies
#-----------------------------------------------------------------------------
# KWStyle
if(${PROJECT_NAME}_USE_KWSTYLE)
  include(ConfigKWStyle)
endif()

#--------------------------------------------------------------------------
# Add Source code subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Base/Core)
add_subdirectory(Base/Scene)
add_subdirectory(Base/SimulationManager)

#--------------------------------------------------------------------------
# Export Targets
#--------------------------------------------------------------------------
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAMESPACE)
set(PROJECT_NAMESPACE "${PROJECT_NAMESPACE}::")

export(EXPORT ${PROJECT_NAME}_TARGETS
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAMESPACE}
  )

install(EXPORT ${PROJECT_NAME}_TARGETS
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAMESPACE}
  COMPONENT Development
  DESTINATION ${${PROJECT_NAME}_INSTALL_SHARE_DIR}
  )

#--------------------------------------------------------------------------
# Add Examples subdirectories
#--------------------------------------------------------------------------
add_subdirectory(Examples/Sandbox)
