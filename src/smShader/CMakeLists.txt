
simmedtk_add_library(smShader
  SOURCES
    smShader.cpp
    smSceneTextureShader.cpp
    smMetalShader.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smShader/smMetalShader.h
    ${CMAKE_SOURCE_DIR}/include/smShader/smSceneTextureShader.h
    ${CMAKE_SOURCE_DIR}/include/smShader/smShader.h
)

target_link_libraries(smShader
  PRIVATE
    smCore
)
