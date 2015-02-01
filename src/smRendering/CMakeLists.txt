
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
    Qt4::QtGui
    Qt4::QtOpenGL
    QGLViewer::QGLViewer
)
get_target_property(inc_prop glfw::glfw INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(link_prop glfw::glfw INTERFACE_LINK_LIBRARIES)
message("INTERFACE_INCLUDE_DIRECTORIES=${inc_prop}")
message("INTERFACE_LINK_LIBRARIES=${link_prop}")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smRendering
    PRIVATE
      -std=c++11
  )
endif()
