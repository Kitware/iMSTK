cmake_minimum_required(VERSION 3.1)

if (POLICY CMP0058)
  cmake_policy(SET CMP0058 NEW)
endif()
project(SimMedTK)

#-----------------------------------------------------------------------------
# Update CMake module path
#
list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/SuperBuild")

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Include macros
#
include(SimMedTKMacros)

#-----------------------------------------------------------------------------
# Library mode: SHARED (default in Linux/Mac) or STATIC (Required in Win32)
#
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
if(WIN32 AND MSVC)
  # TODO: In windows iMSTK needs to build static libraries but still link to
  # shared externals. Add logic to take this into consideration.
  # set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
endif()
set(SimMedTK_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
mark_as_advanced(BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#
option(SimMedTK_SUPERBUILD
    "Build SimMedTK and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(SimMedTK_SUPERBUILD)

#-----------------------------------------------------------------------------
# Devices options
#
option(SimMedTK_USE_PHANTOM_OMNI "Use the phantom omni device." OFF)
option(SimMedTK_USE_ADU "Use the adu device." OFF)
option(SimMedTK_USE_NIUSB6008 "Use the NIUSB6008 device." OFF)
option(SimMedTK_USE_FALCON "Use the Falcon device." OFF)

#-----------------------------------------------------------------------------
# VRPN options
option(USE_VRPN_CLIENT "Use VRPN in client mode." ON)
option(USE_VRPN_SERVER "Use VRPN in server mode." ON)

if(SimMedTK_USE_PHANTOM_OMNI)
    set(USE_VRPN_SERVER ON CACHE BOOL "Force the use of VRPN servers." FORCE)
endif()


#-----------------------------------------------------------------------------
# Oculus Option - Disabled by default
#
option(SimMedTK_USE_OCULUS "Build SimMedTK with Oculus Support" OFF)

#-----------------------------------------------------------------------------
# Output directories - this is where built library and executable
# files will be placed after building but prior to install.  The
# necessary variables change between single and multi configuration
# build systems, so it is necessary to handle both cases on a
# conditional basis.
#
include(SimMedTKOutputDirectories)

#-----------------------------------------------------------------------------
# Prerequisites
#
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()

#-----------------------------------------------------------------------------
# SimMedTK version number.  An even minor number corresponds to releases.
#
set(SimMedTK_VERSION_MAJOR 0)
set(SimMedTK_VERSION_MINOR 0)
set(SimMedTK_VERSION_PATCH 1)
set(SimMedTK_VERSION "${SimMedTK_VERSION_MAJOR}.${SimMedTK_VERSION_MINOR}.${SimMedTK_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Testing
#
option(BUILD_TESTING "Test the project" ON)

#-----------------------------------------------------------------------------
# Documentation options
#
option(SimMedTK_ENABLE_DOCUMENTATION
  "Include targets for Doxygen- and Sphinx-generated documentation" OFF)
if (SimMedTK_ENABLE_DOCUMENTATION)
  find_package(Doxygen)
  find_package(Sphinx)
endif()

#-----------------------------------------------------------------------------
# Coverage
#
option(WITH_COVERAGE "Enable/Disable coverage" OFF)

#-----------------------------------------------------------------------------
# Set coverage Flags
#
if(WITH_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(coverage_flags "-g -fprofile-arcs -ftest-coverage -O0 -DNDEBUG")
    set(COVERAGE_CXX_FLAGS ${coverage_flags})
    set(COVERAGE_C_FLAGS ${coverage_flags})
  endif()
endif()

#-----------------------------------------------------------------------------
# Additional CXX/C Flags
#
set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags")
mark_as_advanced(ADDITIONAL_C_FLAGS)
set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags")
mark_as_advanced(ADDITIONAL_CXX_FLAGS)

#-----------------------------------------------------------------------------
# SimMedTK C/CXX Flags
#
set(SimMedTK_C_FLAGS "${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}")
set(SimMedTK_CXX_FLAGS "${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings")

  if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(cflags "${cflags} -D_FORTIFY_SOURCE=2")
  endif()

  if(MINGW)
    # suppress warnings about auto imported symbols
    set(SimMedTK_CXX_FLAGS "-Wl,--enable-auto-import ${SimMedTK_CXX_FLAGS}")
  endif()

  set(SimMedTK_C_FLAGS "${cflags} ${SimMedTK_C_FLAGS}")
  set(SimMedTK_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${SimMedTK_CXX_FLAGS}")
endif()

if(MSVC)
  set(msvc_suppressed_warnings
  # C++ exception specification ignored except to indicate a function is not
  # __declspec(nothrow)
    "/wd4290"
  )
  set(msvc_other_flags
  # Enable C++ Exceptions
    "/EHsc"
  )
  set(SimMedTK_CXX_FLAGS
    "${SimMedTK_CXX_FLAGS} ${msvc_suppressed_warnings} ${msvc_other_flags}")
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-DWINDOWS_EXTRA_LEAN)
  configure_msvc_runtime()
endif()

#-----------------------------------------------------------------------------
# Superbuild script
#
if(SimMedTK_SUPERBUILD)
  include(SuperBuild)
  return()
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

#-----------------------------------------------------------------------------
# Find required packages
#
find_package(PThreads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SFML REQUIRED)
find_package(Assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(VegaFEM REQUIRED CONFIG)
find_package(ThreadPool REQUIRED)
find_package(VTK REQUIRED)

include(${VTK_USE_FILE})

if(SimMedTK_USE_OCULUS)
  find_package(Oculus REQUIRED)
  include_directories(SYSTEM "${Oculus_INCLUDE_DIRS}")
  if(NOT WIN32)
    find_package(X11 REQUIRED)
  endif()
endif()

#-----------------------------------------------------------------------------
# Testing framework
#
if(BUILD_TESTING)
  include(SimMedTKTesting)
endif()

if(SimMedTK_USE_PHANTOM_OMNI)
  find_package(OpenHaptics)
endif()

if(SimMedTK_USE_PHANTOM_OMNI)
  find_package(OpenHaptics REQUIRED)
endif()

if(USE_VRPN_CLIENT)
  set(components quat)
  if(USE_VRPN_SERVER)
    list(APPEND components vrpnserver Libusb1)
  endif()
  if(SimMedTK_USE_PHANTOM_OMNI)
    list(APPEND components vrpn_phantom)
  endif()
  if(SimMedTK_USE_FALCON)
    list(APPEND components LibNifalcon)
  endif()
  find_package(VRPN REQUIRED COMPONENTS ${components})
  include_directories(SYSTEM ${VRPN_INCLUDE_DIR})
endif()

if(SimMedTK_USE_ADU)
  find_package(ADU REQUIRED)
  include_directories(SYSTEM "${ADU_INCLUDE_DIR}")
endif()

if(SimMedTK_USE_NIUSB6008)
  find_package(NIDAQ REQUIRED)
  include_directories(SYSTEM "${NIDAQ_INCLUDE_DIR}")
endif()

include_directories(SYSTEM "${EIGEN3_INCLUDE_DIR}")
include_directories(SYSTEM "${SFML_INCLUDE_DIRS}")
include_directories(SYSTEM "${ThreadPool_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# SimMedTK_SUPERBUILD_BINARY_DIR
#
# If SimMedTK_SUPERBUILD_BINARY_DIR isn't defined, it means SimMedTK is *NOT* build using
# Superbuild.
# In that case, SimMedTK_SUPERBUILD_BINARY_DIR should default to SimMedTK_BINARY_DIR
#
if(NOT DEFINED SimMedTK_SUPERBUILD_BINARY_DIR)
  set(SimMedTK_SUPERBUILD_BINARY_DIR ${SimMedTK_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags
#
set(CMAKE_CXX_FLAGS ${SimMedTK_CXX_FLAGS} CACHE STRING "CMake CXX Flags" FORCE)
set(CMAKE_C_FLAGS ${SimMedTK_C_FLAGS} CACHE STRING "CMake C Flags" FORCE)

set(SimMedTK_exports
  Collision
  ContactHandling
  Core
  Devices
  Event
  External
  Geometry
  IO
  Mesh
  Rendering
  Simulators
  Solvers
  TimeIntegrators
  VTKRendering
  VirtualTools
  SceneModels
  SimulationManager
  )

if(SimMedTK_USE_OCULUS)
  list(APPEND SimMedTK_exports RenderingOculus)
endif()

add_subdirectory(Collision)
add_subdirectory(ContactHandling)
add_subdirectory(Core)
add_subdirectory(Devices)
add_subdirectory(Event)
add_subdirectory(Examples)
add_subdirectory(External)
add_subdirectory(Geometry)
add_subdirectory(IO)
add_subdirectory(Mesh)
add_subdirectory(Rendering)
add_subdirectory(Simulators)
add_subdirectory(SimulationManager)
add_subdirectory(Solvers)
add_subdirectory(SceneModels)
add_subdirectory(TimeIntegrators)
add_subdirectory(VTKRendering)
add_subdirectory(VirtualTools)

################################################################################
# Build documentation
# This also includes tutorials and other documentation that runs
# examples and/or has its source linked to libraries, so it must
# come after those targets have been declared.
#

if (SimMedTK_ENABLE_DOCUMENTATION)
  add_subdirectory(Documentation)
endif()

################################################################################
# Create and install package configuration files.
#

export(PACKAGE SimMedTK)
export(TARGETS
  ${SimMedTK_exports}
  FILE "${CMAKE_CURRENT_BINARY_DIR}/SimMedTKTargets.cmake"
  NAMESPACE mstk::
  EXPORT_LINK_INTERFACE_LIBRARIES
)

configure_file(
  CMake/SimMedTKConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/SimMedTKConfig.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/SimMedTKConfig.cmake"
  DESTINATION "CMake"
  COMPONENT Development
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

set(CPACK_COMPONENTS_ALL Development)
include(CPack)
