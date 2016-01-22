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
#
# VTK
#

set(VTK_VERSION_NUM 6.3)
# set(VTK_TAG v${VTK_VERSION_NUM}.0)
set(VTK_TAG 47425ee694df750be696b0f6ade9a8dc4fe0b652)
set(VTK_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git)

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(VTK_DEPENDENCIES "")

# Include dependent projects if any
iMSTKCheckDependencies(VTK)

set(proj VTK)
if(NOT DEFINED ${proj}_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT:PATH=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
      -DCMAKE_MACOSX_RPATH:BOOL=ON
    )
  endif()

#   message(STATUS "Adding project:${proj}")
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/SuperBuild/${proj}-build
    PREFIX ${CMAKE_BINARY_DIR}/SuperBuild/${proj}${ep_suffix}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_TAG}
    UPDATE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/iMSTK-build
      -DBUILD_SHARED_LIBS:BOOL=${iMSTK_BUILD_SHARED_LIBS}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DVTK_Group_StandAlone:BOOL=OFF
      -DVTK_Group_Rendering:BOOL=OFF
      -DModule_vtkRenderingOpenGL2:BOOL=ON
      -DModule_vtkIOXML:BOOL=ON
      -DModule_vtkIOLegacy:BOOL=ON
      -DModule_vtkIOPLY:BOOL=ON
      -DModule_vtkInteractionStyle:BOOL=ON
      -DModule_vtkRenderingAnnotation:BOOL=ON
      -DModule_vtkInteractionWidgets:BOOL=ON
      -DModule_vtkglew:BOOL=ON
      -DModule_vtkRenderingContext2D:BOOL=ON
      -DVTK_RENDERING_BACKEND:STRING=OpenGL2
      -DVTK_WRAP_PYTHON:BOOL=OFF
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${ep_config_flags}
    DEPENDS
      ${${proj}_DEPENDENCIES}
#     LOG_DOWNLOAD 1            # Wrap download in script to log output
#     LOG_UPDATE 1              # Wrap update in script to log output
#     LOG_CONFIGURE 1           # Wrap configure in script to log output
#     LOG_BUILD 1               # Wrap build in script to log output
#     LOG_TEST 1                # Wrap test in script to log output
#     LOG_INSTALL 1             # Wrap install in script to log output
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/iMSTK-build/lib/cmake/vtk-${VTK_VERSION_NUM})

else()
  iMSTKEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

set(iMSTK_CMAKE_INCLUDE_PATH ${CMAKE_BINARY_DIR}/iMSTK-build/include/vtk-${VTK_VERSION_NUM}/${sep}${iMSTK_CMAKE_INCLUDE_PATH})
list(APPEND iMSTK_SUPERBUILD_EP_ARGS -DVTK_DIR:PATH=${${proj}_DIR})
