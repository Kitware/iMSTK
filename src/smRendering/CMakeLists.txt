
set(OCULUS_VIEWER_SOURCES)
set(OCULUS_VIEWER_INCLUDES)
set(OCULUS_VIEWER_TARGET_LINK_LIBRARIES)

if (SimMedTK_USE_OCULUS)
  list(APPEND OCULUS_VIEWER_SOURCES
    smOculusViewer.cpp)
  list(APPEND OCULUS_VIEWER_INCLUDES
    ${CMAKE_SOURCE_DIR}/include/smRendering/smOculusViewer.h)
  list(APPEND OCULUS_VIEWER_TARGET_LINK_LIBRARIES
    Oculus::Oculus)
endif(SimMedTK_USE_OCULUS)

simmedtk_add_library(smRendering
  SOURCES
    smConfigRendering.cpp
    smFrameBuffer.cpp
    smLight.cpp
    smRenderer.cpp
    smVAO.cpp
    smVBO.cpp
    smViewer.cpp
    ${OCULUS_VIEWER_SOURCES}
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
    ${OCULUS_VIEWER_INCLUDES}
)

target_link_libraries(smRendering
  PRIVATE
    smCore
    smExternal
    smUtilities
    smShader
 PUBLIC
    ${OCULUS_VIEWER_TARGET_LINK_LIBRARIES}
)
