
simmedtk_add_library(smExternal
  SOURCES
    framebufferObject.cpp
    renderbuffer.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smExternal/framebufferObject.h
    ${CMAKE_SOURCE_DIR}/include/smExternal/renderbuffer.h
)

target_link_libraries(smExternal
  PRIVATE
    smCore
)
