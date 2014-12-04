cmake_minimum_required(VERSION 3.0)

project(sofmis)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (MSVC)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-DWINDOWS_EXTRA_LEAN)
  add_definitions(-DGLUT_BUILDING_LIB)
endif ()

macro (sofmis_find_package package)
  if (WIN32)
    # TODO: Set the right variables.
    # TODO: Make this work in Visual Studio and switch between release and debug properly.
    set(CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/release/${package}/include")
  endif (WIN32)

  find_package(${package} ${ARGN})
endmacro ()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)
sofmis_find_package(GLEW REQUIRED)
sofmis_find_package(GLUT REQUIRED)
sofmis_find_package(QGLViewer REQUIRED)
sofmis_find_package(DevIL REQUIRED)
sofmis_find_package(Eigen3 REQUIRED)
sofmis_find_package(Boost REQUIRED
  COMPONENTS
    thread
    system)
sofmis_find_package(Audiere REQUIRED)
sofmis_find_package(OpenHaptics REQUIRED)
sofmis_find_package(V8 REQUIRED)
sofmis_find_package(OpenCV REQUIRED)
sofmis_find_package(ASSIMP MODULE REQUIRED)

if(BUILD_EXTERNAL_DEVICES)
  sofmis_find_package(ADU REQUIRED)
  sofmis_find_package(ATC REQUIRED)
  sofmis_find_package(NIDAQ REQUIRED)
endif(BUILD_EXTERNAL_DEVICES)

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
include_directories(SYSTEM "${OPENHAPTICS_INCLUDE_DIR}")
include_directories(SYSTEM "${V8_INCLUDE_DIR}")
include_directories(SYSTEM "${OpenCV_INCLUDE_DIRS}")
include_directories(SYSTEM "${ASSIMP_INCLUDE_DIRS}")

if(BUILD_EXTERNAL_DEVICES)
  include_directories(SYSTEM "${ADU_INCLUDE_DIR}")
  include_directories(SYSTEM "${ATC_INCLUDE_DIR}")
  include_directories(SYSTEM "${NIDAQ_INCLUDE_DIR}")
endif(BUILD_EXTERNAL_DEVICES)

add_subdirectory(src)
