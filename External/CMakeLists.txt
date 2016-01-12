simmedtk_add_library(External
  SOURCES
    FrameBufferObject/FrameBufferObject.cpp
    FrameBufferObject/RenderBuffer.cpp
  PUBLIC_HEADERS
    FrameBufferObject/FrameBufferObject.h
    FrameBufferObject/RenderBuffer.h
    Moller/moller2.h
    Moller/moller.h
    Tree/tree.hh
)

target_compile_options(External
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(External
  PRIVATE
    Core
)
