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

imstk_add_library(Rendering
  SOURCES
    Camera.cpp
    VTKViewer.cpp
    VTKMeshRenderDelegate.cpp
    VTKPlaneRenderDelegate.cpp
    VTKStaticSceneObjectRenderDelegate.cpp
    VTKSceneModelRenderDelegate.cpp
    InitVTKRendering.cpp
    ViewerBase.cpp
  PUBLIC_HEADERS
    Camera.h
    VTKViewer.h
    InitVTKRendering.h
    ViewerBase.h
)

#
# Because of the way oculus SDK is required to build, a new library needs
# to be created to link the runtime.
# This might change in the future.
#
set(OCULUS_LINK_LIBRARIES)
if (iMSTK_USE_OCULUS)
  if (NOT WIN32)
    list(APPEND OCULUS_LINK_LIBRARIES
        ${X11_LIBRARIES}
        ${X11_Xrandr_LIB}
        ${X11_Xinput_LIB}
        ${CMAKE_THREAD_LIBS_INIT})
  endif()
  if (UNIX AND NOT APPLE)
    list(APPEND OCULUS_LINK_LIBRARIES rt)
  endif()

endif(iMSTK_USE_OCULUS)


target_link_libraries(Rendering
  PRIVATE
    Core
    Event
    Mesh
    Simulators
    Geometry
    Devices
    SceneModels
 PUBLIC
    ${VTK_LIBRARIES}
    ${OCULUS_LINK_LIBRARIES}
    vtkglew
    sfml::graphics
    sfml::window
    sfml::system
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
