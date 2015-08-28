simmedtk_add_library(Rendering
  SOURCES
    Camera.cpp
    ConfigRendering.cpp
    FrameBuffer.cpp
    GLUtils.cpp
    MetalShader.cpp
    OpenGLRenderer.cpp
    Shader.cpp
    SceneTextureShader.cpp
    TextureManager.cpp
    VAO.cpp
    VBO.cpp
    OpenGLViewer.cpp
  PUBLIC_HEADERS
    ConfigRendering.h
    CustomRenderer.h
    FrameBuffer.h
    OpenGLRenderer.h
    GLUtils.h
    MetalShader.h
    SceneTextureShader.h
    Shader.h
    TextureManager.h
    VAO.h
    VBO.h
    OpenGLViewer.h
)

target_link_libraries(Rendering
  PRIVATE
    Core
    External
    Event
    Mesh
 PUBLIC
    sfml::window
    sfml::system
)


#
# Because of the way oculus SDK is required to build, a new library needs
# to be created to link the runtime.
# This might change in the future.
#
if (SimMedTK_USE_OCULUS)

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

  simmedtk_add_library(RenderingOculus
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
endif(SimMedTK_USE_OCULUS)
