
simmedtk_add_library(smShader
  SOURCES
    smShader.cpp
    SceneTextureShader.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smShader/metalShader.h
    ${CMAKE_SOURCE_DIR}/include/smShader/SceneTextureShader.h
    ${CMAKE_SOURCE_DIR}/include/smShader/smShader.h
)

target_link_libraries(smShader
  PRIVATE
    smCore
)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smShader
    PRIVATE
      -std=c++11
  )
endif()
