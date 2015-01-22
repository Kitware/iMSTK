cmake_minimum_required(VERSION 3.0)

project(SimMedTK)
include(CTest)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(SimMedTK_BUILD_SHARED_LIBS BUILD_SHARED_LIBS)
mark_as_advanced(BUILD_SHARED_LIBS)

list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(SimMedTKMacros)

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
# Prerequisites
#-----------------------------------------------------------------------------
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()

find_package(OpenGL REQUIRED)
simmedtk_find_package(GLEW REQUIRED)
simmedtk_find_package(GLUT REQUIRED)

#-----------------------------------------------------------------------------
# Superbuild script
#
if(SimMedTK_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/SuperBuild/SuperBuild.cmake")
  return()
endif()

if (MSVC)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-DWINDOWS_EXTRA_LEAN)
  add_definitions(-DGLUT_BUILDING_LIB)
endif ()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

if(WIN32)
  set(THREADS_USE_PTHREADS_WIN32 1)
endif(WIN32)
simmedtk_find_package(PThreads REQUIRED)
if(THREADS_FOUND)
  include_directories(${THREADS_PTHREADS_INCLUDE_DIR})
endif()

find_package(Qt4 REQUIRED QtOpenGL QtCore QtGui)

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
simmedtk_find_package(OpenHaptics)
simmedtk_find_package(V8 REQUIRED)

find_package(VegaFEM REQUIRED CONFIG)
find_package(glfw3 REQUIRED CONFIG)
find_package(Assimp REQUIRED CONFIG)
find_package(Eigen3 REQUIRED)

if(BUILD_EXTERNAL_DEVICES)
  simmedtk_find_package(ADU REQUIRED)
  simmedtk_find_package(NIDAQ REQUIRED)
endif(BUILD_EXTERNAL_DEVICES)

# Device options
option(SIMMEDTK_USE_PHANTOM_OMNI "Use the phantom omni device." ${OPENHAPTICS_FOUND})

# Move these to the libraries which actually use them. This minimizes the number
# of include paths per translation unit and makes it easier to see what requires
# what.

include_directories(SYSTEM
  "${QT_QTCORE_INCLUDE_DIR}"
  "${QT_QTGUI_INCLUDE_DIR}"
  "${QT_QTXML_INCLUDE_DIR}"
  "${QT_QTOPENGL_INCLUDE_DIR}"
  "${QT_HEADERS_DIR}")
if (OPENGL_INCLUDE_DIR)
  include_directories(SYSTEM "${OPENGL_INCLUDE_DIR}")
endif ()
include_directories(SYSTEM "${GLEW_INCLUDE_DIR}")
include_directories(SYSTEM "${GLUT_INCLUDE_DIR}")
include_directories(SYSTEM "${QGLVIEWER_INCLUDE_DIRS}")
include_directories(SYSTEM "${IL_INCLUDE_DIR}")
include_directories(SYSTEM "${EIGEN3_INCLUDE_DIR}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")
include_directories(SYSTEM "${AUDIERE_INCLUDE_DIR}")
if(SIMMEDTK_USE_PHANTOM_OMNI)
  include_directories(SYSTEM "${OPENHAPTICS_INCLUDE_DIR}")
endif()
include_directories(SYSTEM "${V8_INCLUDE_DIR}")
link_directories(${ASSIMP_LIBRARY_DIRS})
include_directories(SYSTEM "${ASSIMP_INCLUDE_DIRS}")
include_directories(SYSTEM "${GLFW3_INCLUDE_DIR}")

if(BUILD_EXTERNAL_DEVICES)
  include_directories(SYSTEM "${ADU_INCLUDE_DIR}")
  include_directories(SYSTEM "${NIDAQ_INCLUDE_DIR}")
endif(BUILD_EXTERNAL_DEVICES)

add_subdirectory(src)
add_subdirectory(examples)
