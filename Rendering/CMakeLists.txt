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
    ConfigRendering.cpp
    FrameBuffer.cpp
    GLUtils.cpp
    MetalShader.cpp
    SceneTextureShader.cpp
    Shader.cpp
    TextureManager.cpp
    VAO.cpp
    VBO.cpp
  PUBLIC_HEADERS
    CustomRenderer.h
    FrameBuffer.h
    GLUtils.h
    MetalShader.h
    SceneTextureShader.h
    Shader.h
    TextureManager.h
    VAO.h
    VBO.h
)

target_link_libraries(Rendering
  PRIVATE
    Core
    External
    Event
    Mesh
 PUBLIC
    ${VTK_LIBRARIES}
    vtkglew
    sfml::graphics
    sfml::window
    sfml::system
)


#
# Because of the way oculus SDK is required to build, a new library needs
# to be created to link the runtime.
# This might change in the future.
#
if (iMSTK_USE_OCULUS)

  set(OCULUS_LINK_LIBRARIES)
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

  imstk_add_library(RenderingOculus
    SOURCES
      OculusViewer.cpp
    PUBLIC_HEADERS
      OculusViewer.h
  )

  target_link_libraries(RenderingOculus
    PRIVATE
      Core
      Rendering
    PUBLIC
      ${OCULUS_LINK_LIBRARIES}
  )
endif(iMSTK_USE_OCULUS)
