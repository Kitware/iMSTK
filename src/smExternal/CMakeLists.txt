
simmedtk_add_library(smExternal
  SOURCES
    framebufferObject.cpp
    renderbuffer.cpp
  PUBLIC_HEADERS
    framebufferObject.h
    renderbuffer.h
)

target_compile_options(smExternal
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(smExternal
  PRIVATE
    smCore
)
