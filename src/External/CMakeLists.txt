
simmedtk_add_library(External
  SOURCES
    framebufferObject.cpp
    renderbuffer.cpp
  PUBLIC_HEADERS
    framebufferObject.h
    renderbuffer.h
)

target_compile_options(External
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(External
  PRIVATE
    Core
)
