simmedtk_add_library(smRendering
  SOURCES
    smCamera.cpp
    smConfigRendering.cpp
    smFrameBuffer.cpp
    smGLUtils.cpp
    smMetalShader.cpp
    smRenderer.cpp
    smShader.cpp
    smSceneTextureShader.cpp
    smTextureManager.cpp
    smVAO.cpp
    smVBO.cpp
    smViewer.cpp
  PUBLIC_HEADERS
    smConfigRendering.h
    smCustomRenderer.h
    smFrameBuffer.h
    smGLRenderer.h
    smGLUtils.h
    smMetalShader.h
    smSceneTextureShader.h
    smShader.h
    smTextureManager.h
    smVAO.h
    smVBO.h
    smViewer.h
)

target_link_libraries(smRendering
  PRIVATE
    smCore
    smExternal
    smEvent
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

  simmedtk_add_library(smRenderingOculus
    SOURCES
      smOculusViewer.cpp
    PUBLIC_HEADERS
      smOculusViewer.h
  )

  target_link_libraries(smRenderingOculus
    PRIVATE
      smCore
      smRendering
    PUBLIC
      ${OCULUS_LINK_LIBRARIES}
  )
endif(SimMedTK_USE_OCULUS)
