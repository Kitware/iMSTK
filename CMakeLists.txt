cmake_minimum_required(VERSION 3.1)

project(SimMedTK)

#-----------------------------------------------------------------------------
# Library mode: SHARED (default) or STATIC
#
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(SimMedTK_BUILD_SHARED_LIBS BUILD_SHARED_LIBS)
mark_as_advanced(BUILD_SHARED_LIBS)

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
# Superbuild Option - Enabled by default
#
option(SimMedTK_SUPERBUILD "Build SimMedTK and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(SimMedTK_SUPERBUILD)

#-----------------------------------------------------------------------------
# Device options
option(SimMedTK_USE_PHANTOM_OMNI "Use the phantom omni device." OFF)
option(SimMedTK_USE_ADU "Use the adu device." OFF)
option(SimMedTK_USE_NIUSB6008 "Use the NIUSB6008 device." OFF)

#-----------------------------------------------------------------------------
# Output directories - this is where built library and executable
# files will be placed after building but prior to install.  The
# necessary variables change between single and multi configuration
# build systems, so it is necessary to handle both cases on a
# conditional basis.
#
set(RUNTIME_DIRNAME bin)
set(LIBRARY_DIRNAME lib)
set(ARCHIVE_DIRNAME lib)
if(WIN32)
  set(LIBRARY_DIRNAME bin)
  set(ARCHIVE_DIRNAME bin)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  foreach(type LIBRARY RUNTIME ARCHIVE)
    # Make sure the directory exists
    if(DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY
        AND NOT EXISTS ${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY})
      message(FATAL_ERROR "SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY is set to a non-existing directory [${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY}]")
    endif()

    if(SimMedTK_SUPERBUILD)
      set(output_dir ${SimMedTK_BINARY_DIR}/${${type}_DIRNAME})
      if(NOT DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY)
        set(SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY ${SimMedTK_BINARY_DIR}/SimMedTK-build/${${type}_DIRNAME})
      endif()
    else()
      if(NOT DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY)
        set(output_dir ${SimMedTK_BINARY_DIR}/${${type}_DIRNAME})
      else()
        set(output_dir ${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY})
      endif()
    endif()
    set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
  endforeach()


else()
  # Multi-configuration is more difficult.  Not only do we need to
  # properly set the output directories, but we also need to
  # identify the "toplevel" directory for each configuration so
  # we can place files, documentation, etc. in the correct
  # relative positions.  Because files may be placed by CMake
  # without a build target to put them in their proper relative build
  # directory position using these paths, we must fully qualify them
  # without using CMAKE_CFG_INTDIR.
  #
  # We define directories that may not be quite "standard"
  # for a particular build tool - for example, native VS2010 projects use
  # another directory to denote CPU type being compiled for - but CMake only
  # supports multi-configuration setups having multiple configurations,
  # not multiple compilers.
  #
  # One additional wrinkle we must watch for here is the case where
  # a multi-configuration setup uses "." for its internal directory -
  # if that's the case, we need to just set the various config output
  # directories to the same value.
  set(CFG_ROOT ${SimMedTK_BINARY_DIR})

  foreach(CFG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
      set(CFG_ROOT ${SimMedTK_BINARY_DIR}/${CFG_TYPE})
    endif()
    string(TOUPPER "${CFG_TYPE}" CFG_TYPE_UPPER)
    foreach(type LIBRARY RUNTIME ARCHIVE)
      if(DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}
          AND NOT EXISTS ${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
        message(FATAL_ERROR "SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} is set to a non-existing directory [${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}}]")
      endif()

      if(SimMedTK_SUPERBUILD)
        set(output_dir ${SimMedTK_BINARY_DIR}/${${type}_DIRNAME})
        if(NOT DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER})
          set(SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} ${SimMedTK_BINARY_DIR}/SimMedTK-build/${${type}_DIRNAME})
        endif()
      else()
        if(NOT DEFINED SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER})
          set(output_dir ${SimMedTK_BINARY_DIR}/${${type}_DIRNAME})
        else()
          set(output_dir ${SimMedTK_CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER}})
        endif()
      endif()
      set(CMAKE_${type}_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} ${output_dir} CACHE INTERNAL "Single output directory for building ${CFG_TYPE} libraries.")
    endforeach()

    if(NOT SimMedTK_BINARY_DIR_${CFG_TYPE_UPPER})
      set(SimMedTK_BINARY_DIR_${CFG_TYPE_UPPER} ${CFG_ROOT})
    endif()

    set(CMAKE_BINARY_DIR_${CFG_TYPE_UPPER} SimMedTK_BINARY_DIR_${CFG_TYPE_UPPER} CACHE INTERNAL "Toplevel binary dir for ${CFG_TYPE} building.")

  endforeach()
endif()

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

#-----------------------------------------------------------------------------
# Testing
#
option(BUILD_TESTING "Test the project" ON)
if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  set(CPP_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  # Configuration for the CMake-generated test driver
  set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "
#include <stdexcept>
#if defined(WIN32)
#  pragma warning(disable : 4996)
#endif")
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
  try
    {")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "
    }
  catch( std::exception & excp )
    {
    fprintf(stderr, \"Test driver caught exception: [%s]\\n\", excp.what());
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    printf(\"Exception caught by the test driver\\n\");
    return EXIT_FAILURE;
    }")
endif()

#-----------------------------------------------------------------------------
# Documentation
#
option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)

set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 CACHE PATH "Where documentation archives should be stored")
mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

#-----------------------------------------------------------------------------
# Attempt to discover Doxygen so that DOXYGEN_EXECUTABLE is set to an appropriate default value
#
FIND_PACKAGE(Doxygen QUIET)

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
    "/wd4290" # C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
  )
  set(msvc_other_flags
    "/EHsc" # Enable C++ Exceptions
  )
  set(SimMedTK_CXX_FLAGS "${SimMedTK_CXX_FLAGS} ${msvc_suppressed_warnings} ${msvc_other_flags}")
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-DWINDOWS_EXTRA_LEAN)
  add_definitions(-DGLUT_BUILDING_LIB)
endif()

#-----------------------------------------------------------------------------
# Superbuild script
#
if(SimMedTK_SUPERBUILD)
  include(SuperBuild)
  return()
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#-----------------------------------------------------------------------------
# Find required packages
#
find_package(PThreads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(Assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Qt4 REQUIRED QtCore QtOpenGL)
find_package(VegaFEM REQUIRED CONFIG)

include(SimMedTKMacros)

simmedtk_find_package(GLUT REQUIRED)
simmedtk_find_package(QGLViewer REQUIRED)
simmedtk_find_package(DevIL REQUIRED)
set(IL_INCLUDE_DIR ${IL_INCLUDE_DIR}/..)

simmedtk_find_package(Boost REQUIRED
  COMPONENTS
    thread
    system)

# do not allow boost to use auto-link library stuff
add_definitions(-DBOOST_ALL_NO_LIB=1)

simmedtk_find_package(Audiere REQUIRED)
simmedtk_find_package(V8 REQUIRED)

if(SimMedTK_USE_PHANTOM_OMNI)
  simmedtk_find_package(OpenHaptics)
endif()

if(SimMedTK_USE_ADU)
  simmedtk_find_package(ADU REQUIRED)
  include_directories(SYSTEM "${ADU_INCLUDE_DIR}")
endif()

if(SimMedTK_USE_NIUSB6008)
  simmedtk_find_package(NIDAQ REQUIRED)
  include_directories(SYSTEM "${NIDAQ_INCLUDE_DIR}")
endif()

include_directories(SYSTEM "${IL_INCLUDE_DIR}")
include_directories(SYSTEM "${EIGEN3_INCLUDE_DIR}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# SimMedTK_SUPERBUILD_BINARY_DIR
#
# If SimMedTK_SUPERBUILD_BINARY_DIR isn't defined, it means SimMedTK is *NOT* build using Superbuild.
# In that case, SimMedTK_SUPERBUILD_BINARY_DIR should default to SimMedTK_BINARY_DIR
#
if(NOT DEFINED SimMedTK_SUPERBUILD_BINARY_DIR)
  set(SimMedTK_SUPERBUILD_BINARY_DIR ${SimMedTK_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags
#
set(CMAKE_CXX_FLAGS ${SimMedTK_CXX_FLAGS} CACHE STRING "CMake C Flags" FORCE)
set(CMAKE_C_FLAGS ${SimMedTK_C_FLAGS} CACHE STRING "CMake CXX Flags" FORCE)

add_subdirectory(src)
add_subdirectory(examples)
