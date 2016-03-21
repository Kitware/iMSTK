cmake_minimum_required(VERSION 3.2)

project(iMSTK VERSION 0.0.1 LANGUAGES C CXX)

#-----------------------------------------------------------------------------
# Update CMake module path & cmake dir
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utilities
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
# Define External dependencies
#-----------------------------------------------------------------------------
# g3log
list(APPEND ${PROJECT_NAME}_DEPENDENCIES g3log)
option(USE_SYSTEM_g3log "Exclude g3log from superbuild and use an existing build." OFF)
mark_as_advanced(USE_SYSTEM_g3log)

# Uncrustify
list(APPEND ${PROJECT_NAME}_DEPENDENCIES Uncrustify)
option(USE_SYSTEM_Uncrustify "Exclude Uncrustify from superbuild and use an existing build." OFF)
mark_as_advanced(USE_SYSTEM_Uncrustify)

#-----------------------------------------------------------------------------
# SUPERBUILD
#-----------------------------------------------------------------------------
option(${PROJECT_NAME}_SUPERBUILD "Build ${PROJECT_NAME} and the projects it depends on." ON)

if(${PROJECT_NAME}_SUPERBUILD)
  # Call CMakeLists.txt in CMake/External which will solve the dependencies
  # and add the External projects, including this one: this top-level
  # CMakeLists.txt will be called back with SUPERBUILD=OFF, to execute
  # the rest of the code below (INNERBUILD), which explains the `return`
  add_subdirectory(CMake/External)
  return()
endif()

#-----------------------------------------------------------------------------
# INNERBUILD
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Find external dependencies
#-----------------------------------------------------------------------------
# g3log
find_package( g3log REQUIRED )
include_directories( ${g3log_INCLUDE_DIR} )

# Uncrustify
include(SetupUncrustifyConfig)

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
