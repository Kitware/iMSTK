simmedtk_add_library(smRendering
  SOURCES
    smConfigRendering.cpp
    smFrameBuffer.cpp
    smLight.cpp
    smRenderer.cpp
    smVAO.cpp
    smVBO.cpp
    smViewer.cpp
    smCamera.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smRendering/smConfigRendering.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smCustomRenderer.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smFrameBuffer.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smGLRenderer.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smLight.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smVAO.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smVBO.h
    ${CMAKE_SOURCE_DIR}/include/smRendering/smViewer.h
)

target_link_libraries(smRendering
  PRIVATE
    smCore
    smExternal
    smUtilities
    smShader
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
      ${CMAKE_SOURCE_DIR}/include/smRendering/smOculusViewer.h
  )

  target_link_libraries(smRenderingOculus
    PRIVATE
      smCore
      smRendering
    PUBLIC
      ${OCULUS_LINK_LIBRARIES}
  )
endif(SimMedTK_USE_OCULUS)
